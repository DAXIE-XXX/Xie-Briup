#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "datacontext.h"
#include "netcontext.h"
#include "systemcontrol.h"
#include "configcontext.h"
#include <QIcon>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 设置窗口图标 - 使用图片路径
    app.setWindowIcon(QIcon(":/images/its.ico"));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // 获取 NetContext 单例
    NetContext *nc = NetContext::getObject();

    // 从配置文件读取服务器信息并连接
    QString serverIp;
    quint16 serverPort;
    ConfigContext::getServerInfo(serverIp, serverPort);
    nc->connectToServer(serverIp, serverPort);

    engine.rootContext()->setContextProperty("dc", DataContext::getObject());
    engine.rootContext()->setContextProperty("nc", NetContext::getObject());
    engine.rootContext()->setContextProperty("sc", SystemControl::getObject());

    engine.loadFromModule("SSClient", "Main");

    return QCoreApplication::exec();
}