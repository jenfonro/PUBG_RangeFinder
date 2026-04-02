#include "app/AppController.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("pubg_rangefinder"));
    QApplication::setApplicationDisplayName(QStringLiteral("PUBG Rangefinder"));
    QApplication::setQuitOnLastWindowClosed(false);

    AppController controller;
    controller.initialize();
    return controller.run();
}
