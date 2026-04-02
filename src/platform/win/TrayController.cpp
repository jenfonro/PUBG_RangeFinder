#include "platform/win/TrayController.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QWidget>

TrayController::TrayController(QWidget* settingsWindow, QObject* parent)
    : QObject(parent),
      settingsWindow_(settingsWindow),
      trayIcon_(new QSystemTrayIcon(this)),
      menu_(new QMenu(settingsWindow)),
      openAction_(nullptr),
      exitAction_(nullptr) {
    trayIcon_->setToolTip(QStringLiteral("PUBG Rangefinder"));
    trayIcon_->setIcon(qApp->style()->standardIcon(QStyle::SP_ComputerIcon));
    setupMenu();

    connect(trayIcon_, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        handleActivated(reason);
    });
}

TrayController::~TrayController() = default;

bool TrayController::isAvailable() const {
    return QSystemTrayIcon::isSystemTrayAvailable();
}

void TrayController::show() {
    trayIcon_->show();
}

void TrayController::hide() {
    trayIcon_->hide();
}

void TrayController::setupMenu() {
    openAction_ = menu_->addAction(QStringLiteral("打开设置"));
    exitAction_ = menu_->addAction(QStringLiteral("退出"));

    connect(openAction_, &QAction::triggered, this, &TrayController::openRequested);
    connect(exitAction_, &QAction::triggered, this, &TrayController::exitRequested);

    trayIcon_->setContextMenu(menu_);
}

void TrayController::handleActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) {
        emit openRequested();
    }
}
