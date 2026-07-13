#include "dbcontext.h"
#include <QSemaphore>
#include "dbexec.h"
#include <QVariant>
#include <QSqlDatabase>

DBContext *DBContext::obj = nullptr;
DBContext::DBContext(QObject *parent)
    : QObject{parent}
{

}

DBContext *DBContext::getObject()
{
    if(obj == nullptr)
        obj = new DBContext;
    return obj;
}

void DBContext::setConfig(int execLimit, QString ip, quint16 port, QString username, QString password, QString dbName)
{
    if(semaphore != nullptr)
        return;
    this->execLimit = execLimit;
    this->ip = ip;
    this->port = port;
    this->username = username;
    this->password = password;
    this->dbName = dbName;
    if(this->execLimit <= 0)
        this->execLimit = 50;
}

void DBContext::initContext()
{
    if(semaphore != nullptr)
        return;
    if(execLimit <= 0)
        return;
    semaphore = new QSemaphore{execLimit};
    for(int i = 0; i < execLimit; i++){
        DBExec *exec = new DBExec(ip, port, username, password,
                                  dbName, QString("con_%1")
                                      .arg(i+1), this);
        if(!exec->db->isOpen()){
            delete exec;
            return;
        }
        exec->setProperty("Acquire", false);
        execList.append(exec);
    }
}

bool DBContext::getState()
{
    if(semaphore == nullptr)
        return false;
    int successCount = 0, errorCount = 0;
    for(int i = 0; i < execList.size(); i++){
        if(execList.at(i)->db->isOpen())
            successCount++;
        else
            errorCount++;
    }
    return successCount == execLimit;
}

QString DBContext::getStateMsg()
{
    if(semaphore == nullptr)
        return QString("Not init.");
    int successCount = 0, errorCount = 0;
    for(int i = 0; i < execList.size(); i++){
        if(execList.at(i)->db->isOpen())
            successCount++;
        else
            errorCount++;
    }
    return QString(
        "DB %4@%5:%6 %7 connect %8. limit:%1, successed:%2, failed:%3.")
        .arg(execLimit).arg(successCount).arg(errorCount)
        .arg(username).arg(ip).arg(port).arg(dbName)
        .arg((execLimit == successCount ? "successed" : "failed"))
        ;
}

DBExec *DBContext::getDBExec()
{
    if(semaphore == nullptr)
        return nullptr;
    semaphore->acquire();
    for(int i = 0; i < execList.size(); i++){
        DBExec *exec = execList.at(i);
        if(exec->property("Acquire").toBool() == false){
            exec->setProperty("Acquire", true);
            return exec;
        }
    }
    return nullptr;
}

void DBContext::releaseDBExec(DBExec *exec)
{
    if(exec == nullptr)
        return;
    if(exec->property("Acquire").toBool() == false){
        return;
    }
    exec->setProperty("Acquire", false);
    semaphore->release();
}







