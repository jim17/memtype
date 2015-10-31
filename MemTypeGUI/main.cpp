#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "launch.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    Launch launcher;
    QQmlContext *context;
    context = engine.rootContext();
    context->setContextProperty("Launcher", &launcher);

    return app.exec();
}
