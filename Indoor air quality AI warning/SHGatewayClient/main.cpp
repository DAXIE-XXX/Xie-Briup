#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginDialog loginDialog;
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow w;
    w.setServerInfo(loginDialog.getServerIP(), loginDialog.getServerPort());
    w.show();

    // 自动连接服务器
    w.connectToServer();

    // 连接成功后，MainWindow的onConnected会自动发送注册和阈值

    return a.exec();
}