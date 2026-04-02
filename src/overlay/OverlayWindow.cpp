#include "overlay/OverlayWindow.h"

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

OverlayWindow::OverlayWindow(QWidget* parent)
    : QWidget(parent),
      featureVisible_(false),
      featureEnabledState_(false),
      showCrosshair_(false),
      pointSize_(5),
      lineThickness_(2),
      distanceLabelText_(),
      altDistanceLabelText_() {
    setWindowTitle(QStringLiteral("PUBG Rangefinder Overlay"));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    resize(800, 600);
}

void OverlayWindow::initializeForPrimaryScreen() {
    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        setGeometry(screen->geometry());
    }
    setClickThrough(true);
    hide();
}

void OverlayWindow::showOverlay() {
    show();
    raise();
}

void OverlayWindow::hideOverlay() {
    hide();
}

void OverlayWindow::setClickThrough(bool enabled) {
#ifdef Q_OS_WIN
    const HWND hwnd = reinterpret_cast<HWND>(winId());
    LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED;
    if (enabled) {
        exStyle |= WS_EX_TRANSPARENT;
    } else {
        exStyle &= ~WS_EX_TRANSPARENT;
    }
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle);
#else
    Q_UNUSED(enabled);
#endif
}

void OverlayWindow::setFeatureVisible(bool enabled) {
    featureVisible_ = enabled;
    updateVisibility();
    update();
}

void OverlayWindow::setFeatureEnabledState(bool enabled) {
    featureEnabledState_ = enabled;
    update();
}

void OverlayWindow::setShowCrosshair(bool enabled) {
    showCrosshair_ = enabled;
    updateVisibility();
    update();
}

void OverlayWindow::setPointSize(int pointSize) {
    pointSize_ = pointSize > 0 ? pointSize : 5;
    update();
}

void OverlayWindow::setLineThickness(int lineThickness) {
    lineThickness_ = lineThickness > 0 ? lineThickness : 2;
    update();
}

void OverlayWindow::setMarkPoints(const QVector<QPoint>& points) {
    markPoints_ = points;
    updateVisibility();
    update();
}

void OverlayWindow::setAltSurveyPoints(const QVector<QPoint>& points) {
    altSurveyPoints_ = points;
    updateVisibility();
    update();
}

void OverlayWindow::clearMarkPoints() {
    markPoints_.clear();
    updateVisibility();
    update();
}

void OverlayWindow::clearAltSurveyPoints() {
    altSurveyPoints_.clear();
    updateVisibility();
    update();
}

void OverlayWindow::setDistanceLabelText(const QString& text) {
    distanceLabelText_ = text;
    update();
}

void OverlayWindow::setAltDistanceLabelText(const QString& text) {
    altDistanceLabelText_ = text;
    update();
}

void OverlayWindow::clearDistanceLabel() {
    distanceLabelText_.clear();
    update();
}

void OverlayWindow::clearAltDistanceLabel() {
    altDistanceLabelText_.clear();
    update();
}

void OverlayWindow::drawDistanceLabel(QPainter& painter, const QVector<QPoint>& points, const QString& labelText) {
    if (points.size() < 2 || labelText.isEmpty()) {
        return;
    }

    const QPoint p1 = points.at(0);
    const QPoint p2 = points.at(1);
    const QPoint midPoint((p1.x() + p2.x()) / 2, (p1.y() + p2.y()) / 2);

    QFont font = painter.font();
    font.setPointSize(11);
    font.setBold(true);
    painter.setFont(font);

    QFontMetrics metrics(font);
    const QSize textSize = metrics.size(Qt::TextSingleLine, labelText);
    QRect labelRect(midPoint.x() - textSize.width() / 2 - 12,
                    midPoint.y() - textSize.height() / 2 - 6,
                    textSize.width() + 24,
                    textSize.height() + 12);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(12, 12, 12, 220));
    painter.drawRoundedRect(labelRect, 8, 8);

    painter.setPen(QColor(64, 255, 128, 235));
    painter.drawText(labelRect, Qt::AlignCenter, labelText);
}

void OverlayWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    if (!featureVisible_ && !showCrosshair_) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QColor(20, 55, 85, 230));
    painter.setBrush(Qt::NoBrush);
    painter.drawText(QRect(24, 20, 320, 32),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     featureEnabledState_ ? QStringLiteral("功能状态：开启")
                                          : QStringLiteral("功能状态：关闭"));

    if (featureVisible_ && !markPoints_.isEmpty()) {
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor(36, 135, 255, 220), lineThickness_));
        painter.setBrush(QColor(36, 135, 255, 220));

        for (int i = 1; i < markPoints_.size(); ++i) {
            painter.drawLine(markPoints_.at(i - 1), markPoints_.at(i));
        }
        painter.setPen(Qt::NoPen);
        for (const QPoint& point : markPoints_) {
            painter.drawEllipse(point, pointSize_, pointSize_);
        }

        drawDistanceLabel(painter, markPoints_, distanceLabelText_);
    }

    if (featureVisible_ && !altSurveyPoints_.isEmpty()) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(36, 135, 255, 220));
        for (const QPoint& point : altSurveyPoints_) {
            painter.drawEllipse(point, pointSize_, pointSize_);
        }
        drawDistanceLabel(painter, altSurveyPoints_, altDistanceLabelText_);
    }

    if (showCrosshair_) {
        const QPoint center = rect().center();
        painter.setBrush(QColor(255, 64, 64, 220));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center, pointSize_, pointSize_);
    }
}

void OverlayWindow::updateVisibility() {
    if ((featureVisible_ && (!markPoints_.isEmpty() || !altSurveyPoints_.isEmpty())) || showCrosshair_) {
        showOverlay();
    } else {
        hideOverlay();
    }
}
