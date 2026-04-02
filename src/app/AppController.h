#pragma once

#include <QObject>
#include <QPoint>
#include <QString>
#include <QVector>

#include "config/UiSettingsDraft.h"

class MainWindow;
class OverlayWindow;
class TrayController;
class GlobalHotkeyManager;

class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;

    void initialize();
    int run();

private:
    enum class SurveyMode {
        None,
        MarkLine,
        CenterLine,
        AltNoLine,
    };

    void applyDraft(const UiSettingsDraft& draft);
    void appendMarkPoint(const QPoint& point, bool drawLine);
    void appendCenterSurveyPoint(const QPoint& point);
    void adjustZoomLevel(int delta);
    void clearAllSurveyData();
    void closeSurvey();
    double computePixelsPer100m() const;
    double computeDistanceMeters(const QVector<QPoint>& points) const;
    void handleAltSurveyBindingReleased();
    void handleMarkBindingActivated();
    void handleMarkBindingReleased();
    void refreshDistanceLabel();
    void toggleFeatureVisible();
    void showMainWindow();
    void requestQuit();

    MainWindow* mainWindow_;
    OverlayWindow* overlayWindow_;
    TrayController* trayController_;
    GlobalHotkeyManager* hotkeyManager_;
    UiSettingsDraft draft_;
    bool featureVisible_;
    int primaryScreenHeight_;
    int currentZoomLevel_;
    double currentPixelsPer100m_;
    QVector<QPoint> markPoints_;
    QVector<QPoint> altSurveyPoints_;
    bool markSessionActive_;
    SurveyMode currentSurveyMode_;
};
