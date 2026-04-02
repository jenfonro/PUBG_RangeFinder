#pragma once

#include <QString>
#include <QVector>
#include <QWidget>

class OverlayWindow : public QWidget {
    Q_OBJECT

public:
    explicit OverlayWindow(QWidget* parent = nullptr);

    void initializeForPrimaryScreen();
    void showOverlay();
    void hideOverlay();
    void setClickThrough(bool enabled);
    void setFeatureVisible(bool enabled);
    void setFeatureEnabledState(bool enabled);
    void setShowCrosshair(bool enabled);
    void setPointSize(int pointSize);
    void setLineThickness(int lineThickness);
    void setMarkPoints(const QVector<QPoint>& points);
    void setAltSurveyPoints(const QVector<QPoint>& points);
    void setDistanceLabelText(const QString& text);
    void setAltDistanceLabelText(const QString& text);
    void clearMarkPoints();
    void clearAltSurveyPoints();
    void clearDistanceLabel();
    void clearAltDistanceLabel();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void updateVisibility();
    void drawDistanceLabel(QPainter& painter, const QVector<QPoint>& points, const QString& labelText);

    bool featureVisible_;
    bool featureEnabledState_;
    bool showCrosshair_;
    int pointSize_;
    int lineThickness_;
    QVector<QPoint> markPoints_;
    QVector<QPoint> altSurveyPoints_;
    QString distanceLabelText_;
    QString altDistanceLabelText_;
};
