#include <QCoreApplication>
#include "systemcontrol.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    SystemControl sc;
    sc.systemInit();
    sc.systemStart();
    return QCoreApplication::exec();
}
