#include <QApplication>  // Qt Charts 内部需要 QApplication 初始化 Widget 子系统
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include "sensorcontroller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("GatewayClient");
    app.setOrganizationName("SmartHome");

    // 创建业务逻辑控制器（暴露给 QML）
    SensorController controller;

    // 加载 QML 界面
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("controller", &controller);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
