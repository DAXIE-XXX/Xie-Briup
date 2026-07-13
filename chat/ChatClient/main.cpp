#include "loginpage.h"
#include "configcontext.h"
#include "netcontext.h"
#include <QApplication>
#include "edituserinfopage.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString ip;
    quint16 port;
    ConfigContext::getServerInfo(ip, port);
    NetContext::getObject()->connectToServer(ip, port);
    LoginPage::getObject()->show();
    EditUserInfoPage::getObject();
    return a.exec();
}
