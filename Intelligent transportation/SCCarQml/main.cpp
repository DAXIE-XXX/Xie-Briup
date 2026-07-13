#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "datacontext.h"
#include <QQmlContext>
#include "netcontext.h"
#include "systemcontrol.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    SystemControl::getObject()->systemInit();
    SystemControl::getObject()->systemStart();

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
    engine.loadFromModule("SCCar", "Main");

    return QCoreApplication::exec();
}




