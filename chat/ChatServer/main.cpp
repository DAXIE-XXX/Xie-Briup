#include <QCoreApplication>
#include "systemcontrol.h"
#include <QStandardPaths>
#include "dbcontext.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString customPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString temp;
    temp.append(argv[0]);
    int index = temp.lastIndexOf('/');
    if(index < 0)
        index = temp.lastIndexOf('\\');
    temp = temp.mid(index+1);
    customPath += '/';
    customPath += temp.split('.').at(0);
    customPath += ".ini";
    SystemControl sc;
    sc.systemInit(customPath);
    sc.systemStart();

    return QCoreApplication::exec();
}
