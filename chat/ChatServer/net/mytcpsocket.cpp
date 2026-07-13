#include "mytcpsocket.h"
#include <QJsonObject>
#include <QJsonDocument>
#include "dbcontext.h"
#include "dbexec.h"
#include <QCryptographicHash>
#include <QDateTime>
#include "mqttcontext.h"

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, &MyTcpSocket::readyRead,
            this, &MyTcpSocket::readyReadSlot);
}

void MyTcpSocket::readyReadSlot()
{
    QByteArray data = this->readAll();
    handleData(data);
}

void MyTcpSocket::handleData(QByteArray &data)
{
    int count = 0;
    for(int i = 0; i < data.length(); i++){
        if(data.at(i) == '{')
            count++;
        else if(data.at(i) == '}'){
            count--;
            if(count == 0){
                QByteArray frame = data.mid(0, i+1);
                handleFrame(frame);
                if(frame.length() == data.length())
                    return;
                data = data.mid(i+1);
                i = -1;
            }
        }
    }
}

void MyTcpSocket::handleFrame(const QByteArray &frame)
{
    QJsonObject rf = QJsonDocument::fromJson(frame).object();
    int type = rf.value("type").toInt();
    if(type == 1001){
        handleRegister(rf);
        return;
    }
    else if(type == 1002){
        handleLogin(rf);
        return;
    }
    else if(username.isEmpty()){
        QJsonObject reObj;
        reObj.insert("type", 8000);
        write(QJsonDocument(reObj).toJson());
        return;
    }

    if(type == 1003)
        handleGetUserInfo();
    else if(type == 1004)
        handleSetUserInfo(rf);
    else if(type == 1007)
        handleSetAvatar(rf);
    else if(type == 1005)
        handleUserChangePassword(rf);
    else if(type == 1006)
        handleLogout();
    else if(type == 2001)
        handleFindUser(rf);
    else if(type == 2002)
        handleFriendAddRequest(rf);
    else if(type == 2003)
        handleGetFriendRequestList();
    else if(type == 2004)
        handleAcceptFriendRequest(rf);
    else if(type == 2005)
        handleGetFriendList();
    else if(type == 3001)
        handleChatUser(rf);
    else if(type == 4001)
        handleGroupCreate(rf);
    else if(type == 4002)
        handleGroupGetList();
    else if(type == 3002)
        handleGroupChatMsg(rf);
    else if(type == 4003)
        handleGroupAddMember(rf);
    else if(type == 4004)
        handleGroupRemoveMember(rf);
    else if(type == 4005)
        handleGroupGetMembers(rf);
    else if(type == 4006)
        handleGroupSetAdmins(rf);
    else if(type == 4007)
        handleGroupDissolve(rf);
    else if(type == 4008)
        handleGroupExit(rf);
    else if(type == 4009)
        handleGroupGetAdmins(rf);
    else if(type == 4010)
        handleGroupRename(rf);
    else if(type == 4011)
        handleGroupTransfer(rf);
    else if(type == 2007)
        handleFriendBlock(rf);
    else if(type == 2008)
        handleFriendUnblock(rf);
    else if(type == 2009)
        handleFriendGroupCreate(rf);
    else if(type == 2010)
        handleFriendGroupDelete(rf);
    else if(type == 2011)
        handleFriendGroupRename(rf);
    else if(type == 2012)
        handleFriendGroupGetList();
    else if(type == 2013)
        handleFriendSetGroup(rf);
    else if(type == 2014)
        handleFriendSetRemark(rf);
    else if(type == 4012)
        handleGroupSetRemark(rf);
}

void MyTcpSocket::handleRegister(const QJsonObject &obj)
{
    QString username = obj.value("username").toString();
    QString password = obj.value("password").toString();
    password = QCryptographicHash::hash(password.toLocal8Bit(),
                             QCryptographicHash::Sha256)
        .toHex();
    bool re = false;
    DBExec *exec = DBContext::getObject()->getDBExec();
    if(exec != nullptr){
        re = exec->userInsert(username, password);
        DBContext::getObject()->releaseDBExec(exec);
    }

    QJsonObject reObj;
    reObj.insert("type", 1001);
    reObj.insert("username", username);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleLogin(const QJsonObject &obj)
{
    QString username = obj.value("username").toString();
    QString password = obj.value("password").toString();
    password = QCryptographicHash::hash(password.toLocal8Bit(),
                                        QCryptographicHash::Sha256)
                   .toHex();
    bool re = false;
    DBExec *exec = DBContext::getObject()->getDBExec();
    if(exec != nullptr){
        re = exec->userSelect(username, password);
        if(re){
            this->username = username;
            this->userId = exec->userGetId(username);
        }
        DBContext::getObject()->releaseDBExec(exec);
    }

    QJsonObject reObj;
    reObj.insert("type", 1002);
    reObj.insert("username", username);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGetUserInfo()
{
    QJsonObject reObj;
    DBExec *exec = DBContext::getObject()->getDBExec();
    if(exec != nullptr){
        reObj = exec->userGetInfo(userId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    reObj.insert("type", 1003);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleSetUserInfo(const QJsonObject &obj)
{
    QString name = obj.value("name").toString();
    QString phone = obj.value("phone").toString();
    QString email = obj.value("email").toString();
    QString info = obj.value("info").toString();
    QString birthday = obj.value("birthday").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        re = exec->userSetInfo(userId, name, phone, email, info, birthday);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 1004);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleSetAvatar(const QJsonObject &obj)
{
    QString avatarBase64 = obj.value("avatar").toString();
    QByteArray avatar = QByteArray::fromBase64(avatarBase64.toUtf8());
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        re = exec->userSetAvatar(userId, avatar);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 1007);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleUserChangePassword(const QJsonObject &obj)
{
    QString oldPassword = obj.value("old_password").toString();
    QString newPassword = obj.value("new_password").toString();
    oldPassword = QCryptographicHash::hash(oldPassword.toLocal8Bit(),
                                           QCryptographicHash::Sha256)
                      .toHex();
    newPassword = QCryptographicHash::hash(newPassword.toLocal8Bit(),
                                           QCryptographicHash::Sha256)
                      .toHex();
    bool re = false;
    DBExec *exec = DBContext::getObject()->getDBExec();
    if(exec != nullptr){
        bool ok = exec->userSelect(username, oldPassword);
        if(ok){
            re = exec->userChangePassword(username, newPassword);
        }
        DBContext::getObject()->releaseDBExec(exec);
    }
    if(re){
        userId = -1;
        username.clear();
    }
    QJsonObject reObj;
    reObj.insert("type", 1005);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleLogout()
{
    username.clear();
    userId = -1;
    QJsonObject reObj;
    reObj.insert("type", 1006);
    reObj.insert("re", true);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleFindUser(const QJsonObject &obj)
{
    QString name = obj.value("name").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    QJsonArray list;
    if(exec != nullptr){
        list = exec->userFind(name, userId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2001);
    reObj.insert("list", list);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleFriendAddRequest(const QJsonObject &obj)
{
    QString toUsername = obj.value("username").toString();
    QString reqMsg = obj.value("req_msg").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec == nullptr){
        QJsonObject reObj;
        reObj.insert("type", 2002);
        reObj.insert("re", false);
        write(QJsonDocument(reObj).toJson());
        return;
    }
    int toId = exec->userGetId(toUsername);
    if(toId < 0){
        DBContext::getObject()->releaseDBExec(exec);
        QJsonObject reObj;
        reObj.insert("type", 2002);
        reObj.insert("re", false);
        write(QJsonDocument(reObj).toJson());
        return;
    }
    bool ok = exec->friendCheck(userId, toId);
    if(ok){
        DBContext::getObject()->releaseDBExec(exec);
        QJsonObject reObj;
        reObj.insert("type", 2002);
        reObj.insert("re", false);
        write(QJsonDocument(reObj).toJson());
        return;
    }
    exec->userAddFriendRequest(userId, toId, reqMsg, QDateTime::currentDateTime()
                               .toString("yyyy-MM-dd hh:mm:ss.zzz"));
    DBContext::getObject()->releaseDBExec(exec);
    QJsonObject reObj;
    reObj.insert("type", 2002);
    reObj.insert("re", true);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGetFriendRequestList()
{
    DBExec *exec = DBContext::getObject()->getDBExec();
    QJsonArray list;
    if(exec != nullptr){
        list = exec->userGetRequestList(userId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2003);
    reObj.insert("list", list);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleAcceptFriendRequest(const QJsonObject &obj)
{
    int reqId = obj.value("req_id").toInt();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re;
    if(exec != nullptr){
        re = exec->userAcceptFriendRequest(reqId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2004);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGetFriendList()
{
    DBExec *exec = DBContext::getObject()->getDBExec();
    QJsonArray list;
    if(exec != nullptr){
        list = exec->userGetFriendListWithGroup(userId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2005);
    reObj.insert("list", list);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleChatUser(const QJsonObject &obj)
{
    QString toUsername = obj.value("to_username").toString();
    int index = obj.value("index").toInt();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec == nullptr){
        QJsonObject reObj;
        reObj.insert("type", 3001);
        reObj.insert("re", false);
        reObj.insert("index", index);
        write(QJsonDocument(reObj).toJson());
        return;
    }
    int toId = exec->userGetId(toUsername);
    if(toId < 0){
        DBContext::getObject()->releaseDBExec(exec);
        QJsonObject reObj;
        reObj.insert("type", 3001);
        reObj.insert("re", false);
        reObj.insert("index", index);
        write(QJsonDocument(reObj).toJson());
        return;
    }
    bool ok = exec->friendCheck(userId, toId);
    if(!ok){
        DBContext::getObject()->releaseDBExec(exec);
        QJsonObject reObj;
        reObj.insert("type", 3001);
        reObj.insert("re", false);
        reObj.insert("index", index);
        write(QJsonDocument(reObj).toJson());
        return;
    }
    bool blocked = exec->friendIsBlocked(toId, userId);
    if(blocked){
        DBContext::getObject()->releaseDBExec(exec);
        QJsonObject reObj;
        reObj.insert("type", 3001);
        reObj.insert("re", false);
        reObj.insert("index", index);
        write(QJsonDocument(reObj).toJson());
        return;
    }
    DBContext::getObject()->releaseDBExec(exec);
    QJsonObject newObj = obj;
    newObj.insert("from_username", username);
    MQTTContext::getObject()
        ->publishValue(
            QString("chat/%1").arg(toUsername),
            QJsonDocument(newObj).toJson(),
            1, false);

    QJsonObject reObj;
    reObj.insert("type", 3001);
    reObj.insert("re", true);
    reObj.insert("index", index);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupCreate(const QJsonObject &obj)
{
    QString groupName = obj.value("group_name").toString();
    int groupId = -1;
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        re = exec->groupCreate(groupName, userId, groupId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4001);
    reObj.insert("re", re);
    if(re)
        reObj.insert("group_id", groupId);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupGetList()
{
    DBExec *exec = DBContext::getObject()->getDBExec();
    QJsonArray list;
    if(exec != nullptr){
        list = exec->groupGetList(userId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4002);
    reObj.insert("list", list);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupChatMsg(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    int index = obj.value("index").toInt();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec == nullptr){
        QJsonObject reObj;
        reObj.insert("type", 3002);
        reObj.insert("re", false);
        reObj.insert("index", index);
        write(QJsonDocument(reObj).toJson());
        return;
    }
    bool isMember = exec->groupCheckMember(groupId, userId);
    if(!isMember){
        DBContext::getObject()->releaseDBExec(exec);
        QJsonObject reObj;
        reObj.insert("type", 3002);
        reObj.insert("re", false);
        reObj.insert("index", index);
        write(QJsonDocument(reObj).toJson());
        return;
    }
    QList<int> memberIds = exec->groupGetMemberIds(groupId);
    DBContext::getObject()->releaseDBExec(exec);

    QJsonObject newObj = obj;
    newObj.insert("from_username", username);
    QByteArray payload = QJsonDocument(newObj).toJson();

    for(int id : memberIds){
        if(id == userId)
            continue;
        DBExec *e = DBContext::getObject()->getDBExec();
        if(e == nullptr)
            continue;
        QString memberName = e->userGetUsername(id);
        DBContext::getObject()->releaseDBExec(e);
        if(memberName.isEmpty())
            continue;
        QString topic = QString("chat/%1").arg(memberName);
        MQTTContext::getObject()->publishValue(topic, QString::fromUtf8(payload), 1, false);
    }

    QJsonObject reObj;
    reObj.insert("type", 3002);
    reObj.insert("re", true);
    reObj.insert("index", index);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupAddMember(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    QString memberUsername = obj.value("username").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        bool isMember = exec->groupCheckMember(groupId, userId);
        if(isMember){
            int memberId = exec->userGetId(memberUsername);
            if(memberId > 0){
                bool alreadyIn = exec->groupCheckMember(groupId, memberId);
                if(!alreadyIn){
                    re = exec->groupAddMember(groupId, memberId);
                }
            }
        }
        DBContext::getObject()->releaseDBExec(exec);
    }
    if(re){
        QJsonObject notify;
        notify.insert("type", 3004);
        notify.insert("group_id", groupId);
        MQTTContext::getObject()
            ->publishValue(
                QString("chat/%1").arg(memberUsername),
                QJsonDocument(notify).toJson(),
                1, false);
    }
    QJsonObject reObj;
    reObj.insert("type", 4003);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupRemoveMember(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    QString memberUsername = obj.value("username").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        if(exec->groupIsAdmin(groupId, userId)){
            int memberId = exec->userGetId(memberUsername);
            if(memberId > 0){
                re = exec->groupRemoveMember(groupId, memberId);
            }
        }
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4004);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupGetMembers(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    DBExec *exec = DBContext::getObject()->getDBExec();
    QJsonArray list;
    if(exec != nullptr){
        list = exec->groupGetMembers(groupId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4005);
    reObj.insert("list", list);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupDissolve(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        re = exec->groupDissolve(groupId, userId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4007);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupExit(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        re = exec->groupExit(groupId, userId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4008);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupGetAdmins(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    DBExec *exec = DBContext::getObject()->getDBExec();
    QJsonArray list;
    if(exec != nullptr){
        list = exec->groupGetAdmins(groupId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4009);
    reObj.insert("list", list);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupSetAdmins(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    QJsonArray arr = obj.value("usernames").toArray();
    QStringList usernames;
    for(int i = 0; i < arr.size(); i++)
        usernames.append(arr.at(i).toString());
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        if(exec->groupIsOwner(groupId, userId)){
            re = exec->groupSetAdmins(groupId, usernames);
        }
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4006);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupRename(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    QString newName = obj.value("new_name").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        if(exec->groupIsAdmin(groupId, userId)){
            re = exec->groupRename(groupId, newName);
        }
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4010);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupTransfer(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    QString newOwnerUsername = obj.value("username").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        if(exec->groupIsOwner(groupId, userId)){
            int newOwnerId = exec->userGetId(newOwnerUsername);
            if(newOwnerId > 0){
                re = exec->groupTransfer(groupId, userId, newOwnerId);
            }
        }
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4011);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleFriendBlock(const QJsonObject &obj)
{
    QString friendUsername = obj.value("username").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        int friendId = exec->userGetId(friendUsername);
        if(friendId > 0){
            re = exec->friendBlock(userId, friendId);
        }
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2007);
    reObj.insert("username", friendUsername);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleFriendUnblock(const QJsonObject &obj)
{
    QString friendUsername = obj.value("username").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        int friendId = exec->userGetId(friendUsername);
        if(friendId > 0){
            re = exec->friendUnblock(userId, friendId);
        }
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2008);
    reObj.insert("username", friendUsername);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleFriendGroupCreate(const QJsonObject &obj)
{
    QString groupName = obj.value("group_name").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    int groupId = -1;
    if(exec != nullptr){
        re = exec->friendGroupCreate(userId, groupName, groupId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2009);
    reObj.insert("re", re);
    if(re){
        reObj.insert("group_id", groupId);
        reObj.insert("group_name", groupName);
    }
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleFriendGroupDelete(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        re = exec->friendGroupDelete(userId, groupId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2010);
    reObj.insert("group_id", groupId);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleFriendGroupRename(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    QString newName = obj.value("new_name").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        re = exec->friendGroupRename(userId, groupId, newName);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2011);
    reObj.insert("group_id", groupId);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleFriendGroupGetList()
{
    DBExec *exec = DBContext::getObject()->getDBExec();
    QJsonArray list;
    if(exec != nullptr){
        list = exec->friendGroupGetList(userId);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2012);
    reObj.insert("list", list);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleFriendSetGroup(const QJsonObject &obj)
{
    QString friendUsername = obj.value("username").toString();
    int groupId = obj.value("group_id").toInt();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        int friendId = exec->userGetId(friendUsername);
        if(friendId > 0){
            bool isFriend = exec->friendCheck(userId, friendId);
            if(isFriend){
                if(groupId == 0){
                    re = exec->friendSetGroup(userId, friendId, groupId);
                } else {
                    bool groupBelongsToUser = exec->friendGroupBelongsToUser(userId, groupId);
                    if(groupBelongsToUser){
                        re = exec->friendSetGroup(userId, friendId, groupId);
                    }
                }
            }
        }
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2013);
    reObj.insert("username", friendUsername);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleFriendSetRemark(const QJsonObject &obj)
{
    QString friendUsername = obj.value("username").toString();
    QString remark = obj.value("remark").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        int friendId = exec->userGetId(friendUsername);
        if(friendId > 0){
            re = exec->friendSetRemark(userId, friendId, remark);
        }
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 2014);
    reObj.insert("username", friendUsername);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}

void MyTcpSocket::handleGroupSetRemark(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    QString remark = obj.value("remark").toString();
    DBExec *exec = DBContext::getObject()->getDBExec();
    bool re = false;
    if(exec != nullptr){
        re = exec->groupSetRemark(userId, groupId, remark);
        DBContext::getObject()->releaseDBExec(exec);
    }
    QJsonObject reObj;
    reObj.insert("type", 4012);
    reObj.insert("group_id", groupId);
    reObj.insert("re", re);
    write(QJsonDocument(reObj).toJson());
}