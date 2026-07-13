#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "datacontext.h"
#include "systemcontrol.h"
#include "netcontext.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    SystemControl::getObject()->system_init();
    SystemControl::getObject()->systen_start();

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.rootContext()->setContextProperty(
        "dc", DataContext::getObject());
    engine.rootContext()->setContextProperty(
        "sc", SystemControl::getObject());

    engine.loadFromModule("SCCrossing", "Main");

    return QCoreApplication::exec();
}
