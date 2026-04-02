#include "app/AppController.h"

#include "overlay/OverlayWindow.h"
#include "platform/win/GlobalHotkeyManager.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QGuiApplication>
#include <QtGlobal>
#include <QScreen>
#include <QString>

#include <cmath>

AppController::AppController(QObject* parent)
    : QObject(parent),
      mainWindow_(nullptr),
      overlayWindow_(nullptr),
      hotkeyManager_(nullptr),
      draft_(defaultUiSettingsDraft()),
      featureVisible_(false),
      primaryScreenHeight_(0),
      currentZoomLevel_(0),
      currentPixelsPer100m_(0.0),
      markPoints_(),
      altSurveyPoints_(),
      markSessionActive_(false),
      currentSurveyMode_(SurveyMode::None) {}

AppController::~AppController() = default;

void AppController::initialize() {
    qApp->setQuitOnLastWindowClosed(true);
    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        primaryScreenHeight_ = screen->geometry().height();
    }

    mainWindow_ = new MainWindow();
    mainWindow_->loadDraft(draft_);

    overlayWindow_ = new OverlayWindow();
    overlayWindow_->initializeForPrimaryScreen();
    applyDraft(draft_);

    hotkeyManager_ = new GlobalHotkeyManager(this);
    connect(hotkeyManager_, &GlobalHotkeyManager::toggleRequested, this, [this]() {
        toggleFeatureVisible();
    });
    connect(hotkeyManager_, &GlobalHotkeyManager::escCloseRequested, this, [this]() {
        closeSurvey();
    });
    connect(hotkeyManager_, &GlobalHotkeyManager::zoomChangedByWheel, this, [this](int deltaSteps) {
        adjustZoomLevel(deltaSteps);
    });
    connect(hotkeyManager_, &GlobalHotkeyManager::markBindingActivated, this, [this]() {
        handleMarkBindingActivated();
    });
    connect(hotkeyManager_, &GlobalHotkeyManager::markPointRequested, this, [this](const QPoint& point) {
        appendMarkPoint(point, true);
    });
    connect(hotkeyManager_, &GlobalHotkeyManager::altRightPointRequested, this, [this](const QPoint& point) {
        appendMarkPoint(point, false);
    });
    connect(hotkeyManager_, &GlobalHotkeyManager::centerSurveyPointRequested, this, [this](const QPoint& point) {
        appendCenterSurveyPoint(point);
    });
    connect(hotkeyManager_, &GlobalHotkeyManager::markBindingReleased, this, [this]() {
        handleMarkBindingReleased();
    });
    connect(hotkeyManager_, &GlobalHotkeyManager::altSurveyBindingReleased, this, [this]() {
        handleAltSurveyBindingReleased();
    });
    hotkeyManager_->applyBindings(draft_.toggleFeature.text, draft_.toggleFeatureAlt.text, draft_.markPoint.text);
    hotkeyManager_->setEscCloseEnabled(draft_.escToClose);
    hotkeyManager_->setAltRightMouseSurveyEnabled(draft_.altRightMouseSurvey);
    hotkeyManager_->setSpaceCenterSurveyEnabled(draft_.spaceCenterSurvey);

    connect(mainWindow_, &MainWindow::settingsApplied, this, [this](const UiSettingsDraft& draft) {
        applyDraft(draft);
    });

    showMainWindow();
}

int AppController::run() {
    return qApp->exec();
}

void AppController::applyDraft(const UiSettingsDraft& draft) {
    draft_ = draft;
    currentPixelsPer100m_ = computePixelsPer100m();

    if (overlayWindow_ == nullptr) {
        return;
    }

    bool ok = false;
    const int pointSize = draft.pointSizeText.toInt(&ok);
    overlayWindow_->setPointSize(ok ? pointSize : 5);
    const int lineThickness = draft.lineThicknessText.toInt(&ok);
    overlayWindow_->setLineThickness(ok ? lineThickness : 2);
    overlayWindow_->setShowCrosshair(draft.showCrosshair);
    overlayWindow_->setFeatureVisible(featureVisible_);
    overlayWindow_->setFeatureEnabledState(featureVisible_);
    overlayWindow_->setMarkPoints(markPoints_);
    overlayWindow_->setAltSurveyPoints(altSurveyPoints_);
    refreshDistanceLabel();

    if (hotkeyManager_ != nullptr) {
        hotkeyManager_->applyBindings(draft.toggleFeature.text, draft.toggleFeatureAlt.text, draft.markPoint.text);
        hotkeyManager_->setEscCloseEnabled(draft.escToClose);
        hotkeyManager_->setAltRightMouseSurveyEnabled(draft.altRightMouseSurvey);
        hotkeyManager_->setSpaceCenterSurveyEnabled(draft.spaceCenterSurvey);
    }
}

void AppController::appendMarkPoint(const QPoint& point, bool drawLine) {
    if (!featureVisible_ || overlayWindow_ == nullptr) {
        return;
    }

    const SurveyMode incomingMode = drawLine ? SurveyMode::MarkLine : SurveyMode::AltNoLine;
    if (currentSurveyMode_ != incomingMode) {
        clearAllSurveyData();
        currentSurveyMode_ = incomingMode;
    }

    QVector<QPoint>& targetPoints = drawLine ? markPoints_ : altSurveyPoints_;
    if (!markSessionActive_) {
        clearAllSurveyData();
        currentSurveyMode_ = incomingMode;
        markSessionActive_ = true;
    }

    if (targetPoints.size() < 2) {
        targetPoints.append(point);
    } else {
        targetPoints[1] = point;
    }

    if (drawLine) {
        overlayWindow_->setMarkPoints(markPoints_);
        refreshDistanceLabel();
    } else {
        overlayWindow_->setAltSurveyPoints(altSurveyPoints_);
        refreshDistanceLabel();
    }
}

void AppController::appendCenterSurveyPoint(const QPoint& point) {
    if (!featureVisible_ || overlayWindow_ == nullptr) {
        return;
    }

    if (currentSurveyMode_ != SurveyMode::CenterLine) {
        clearAllSurveyData();
        currentSurveyMode_ = SurveyMode::CenterLine;
    }

    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        const QRect geometry = screen->geometry();
        const QPoint center = geometry.center();
        altSurveyPoints_.clear();
        overlayWindow_->clearAltSurveyPoints();
        markPoints_.clear();
        markPoints_.append(center);
        markPoints_.append(point);
        markSessionActive_ = true;
        overlayWindow_->setMarkPoints(markPoints_);
        refreshDistanceLabel();
    }
}

void AppController::adjustZoomLevel(int delta) {
    if (!featureVisible_) {
        return;
    }

    const int nextZoomLevel = qBound(0, currentZoomLevel_ + delta, 4);
    if (nextZoomLevel == currentZoomLevel_) {
        return;
    }

    currentZoomLevel_ = nextZoomLevel;
    currentPixelsPer100m_ = computePixelsPer100m();
    refreshDistanceLabel();
}

void AppController::clearAllSurveyData() {
    markSessionActive_ = false;
    currentSurveyMode_ = SurveyMode::None;
    markPoints_.clear();
    altSurveyPoints_.clear();

    if (overlayWindow_ == nullptr) {
        return;
    }

    overlayWindow_->clearMarkPoints();
    overlayWindow_->clearAltSurveyPoints();
    overlayWindow_->clearDistanceLabel();
    overlayWindow_->clearAltDistanceLabel();
}

void AppController::handleMarkBindingActivated() {
    if (!featureVisible_) {
        return;
    }

    clearAllSurveyData();
    currentSurveyMode_ = SurveyMode::MarkLine;
    markSessionActive_ = true;
}

void AppController::handleMarkBindingReleased() {
    if (!featureVisible_) {
        markSessionActive_ = false;
        return;
    }

    if (!markSessionActive_) {
        return;
    }

    if (markPoints_.size() < 2) {
        markPoints_.clear();
        if (overlayWindow_ != nullptr) {
            overlayWindow_->clearMarkPoints();
            overlayWindow_->clearDistanceLabel();
        }
    }

    markSessionActive_ = false;
}

void AppController::handleAltSurveyBindingReleased() {
    if (!featureVisible_) {
        markSessionActive_ = false;
        return;
    }

    if (!markSessionActive_ || currentSurveyMode_ != SurveyMode::AltNoLine) {
        return;
    }

    if (altSurveyPoints_.size() < 2) {
        altSurveyPoints_.clear();
        if (overlayWindow_ != nullptr) {
            overlayWindow_->clearAltSurveyPoints();
            overlayWindow_->clearAltDistanceLabel();
        }
    }

    markSessionActive_ = false;
}

void AppController::closeSurvey() {
    featureVisible_ = false;
    clearAllSurveyData();

    if (overlayWindow_ == nullptr) {
        return;
    }

    overlayWindow_->setFeatureVisible(false);
    overlayWindow_->setFeatureEnabledState(false);
}

void AppController::toggleFeatureVisible() {
    featureVisible_ = !featureVisible_;

    if (overlayWindow_ == nullptr) {
        return;
    }

    overlayWindow_->setFeatureVisible(featureVisible_);
    overlayWindow_->setFeatureEnabledState(featureVisible_);
    if (!featureVisible_) {
        clearAllSurveyData();
    }
}

double AppController::computePixelsPer100m() const {
    bool ok = false;
    const double mapWidth = draft_.mapWidthText.toDouble(&ok);
    const double safeMapWidth = (ok && mapWidth > 0.0) ? mapWidth : 80.0;
    if (primaryScreenHeight_ <= 0) {
        return 0.0;
    }

    if (currentZoomLevel_ == 0) {
        return static_cast<double>(primaryScreenHeight_) / safeMapWidth;
    }

    double zoomFactor = 0.0;
    switch (currentZoomLevel_) {
    case 1:
        zoomFactor = draft_.mapScale1xText.toDouble(&ok);
        if (!ok || zoomFactor <= 0.0) zoomFactor = 2.10526;
        break;
    case 2:
        zoomFactor = draft_.mapScale2xText.toDouble(&ok);
        if (!ok || zoomFactor <= 0.0) zoomFactor = 4.0;
        break;
    case 3:
        zoomFactor = draft_.mapScale3xText.toDouble(&ok);
        if (!ok || zoomFactor <= 0.0) zoomFactor = 8.0;
        break;
    case 4:
        zoomFactor = draft_.mapScale4xText.toDouble(&ok);
        if (!ok || zoomFactor <= 0.0) zoomFactor = 16.0;
        break;
    default:
        return static_cast<double>(primaryScreenHeight_) / safeMapWidth;
    }

    return static_cast<double>(primaryScreenHeight_) * zoomFactor / safeMapWidth;
}

double AppController::computeDistanceMeters(const QVector<QPoint>& points) const {
    if (points.size() < 2 || currentPixelsPer100m_ <= 0.0) {
        return -1.0;
    }

    const QPoint& p1 = points.at(0);
    const QPoint& p2 = points.at(1);
    const double dx = static_cast<double>(p2.x() - p1.x());
    const double dy = static_cast<double>(p2.y() - p1.y());
    const double pixelDistance = std::sqrt(dx * dx + dy * dy);

    bool ok = false;
    const double mapUnit = draft_.mapUnitText.toDouble(&ok);
    const double safeMapUnit = (ok && mapUnit > 0.0) ? mapUnit : 100.0;
    return pixelDistance * safeMapUnit / currentPixelsPer100m_;
}

void AppController::refreshDistanceLabel() {
    if (overlayWindow_ == nullptr) {
        return;
    }

    const double markDistanceMeters = computeDistanceMeters(markPoints_);
    if (markDistanceMeters < 0.0 || markPoints_.size() < 2) {
        overlayWindow_->clearDistanceLabel();
    } else {
        const qint64 roundedDistance = static_cast<qint64>(std::llround(markDistanceMeters));
        overlayWindow_->setDistanceLabelText(QStringLiteral("%1 M").arg(roundedDistance));
    }

    const double altDistanceMeters = computeDistanceMeters(altSurveyPoints_);
    if (altDistanceMeters < 0.0 || altSurveyPoints_.size() < 2) {
        overlayWindow_->clearAltDistanceLabel();
    } else {
        const qint64 roundedDistance = static_cast<qint64>(std::llround(altDistanceMeters));
        overlayWindow_->setAltDistanceLabelText(QStringLiteral("%1 M").arg(roundedDistance));
    }
}

void AppController::showMainWindow() {
    if (mainWindow_ == nullptr) {
        return;
    }
    mainWindow_->showSettingsWindow();
}

void AppController::requestQuit() {
    if (overlayWindow_ != nullptr) {
        overlayWindow_->hideOverlay();
    }
    qApp->quit();
}
