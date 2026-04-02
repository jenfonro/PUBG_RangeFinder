#pragma once

#include <QObject>
#include <QSystemTrayIcon>

class QAction;
class QMenu;
class QWidget;

class TrayController : public QObject {
    Q_OBJECT

public:
    explicit TrayController(QWidget* settingsWindow, QObject* parent = nullptr);
    ~TrayController() override;

    bool isAvailable() const;
    void show();
    void hide();

signals:
    void openRequested();
    void exitRequested();

private:
    void setupMenu();
    void handleActivated(QSystemTrayIcon::ActivationReason reason);

    QWidget* settingsWindow_;
    QSystemTrayIcon* trayIcon_;
    QMenu* menu_;
    QAction* openAction_;
    QAction* exitAction_;
};
