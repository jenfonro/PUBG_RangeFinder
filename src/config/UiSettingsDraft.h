#pragma once

#include <QString>

struct HotkeyDisplayBinding {
    QString text;
};

struct UiSettingsDraft {
    HotkeyDisplayBinding toggleFeature;
    HotkeyDisplayBinding toggleFeatureAlt;
    HotkeyDisplayBinding markPoint;
    QString pointSizeText;
    QString lineThicknessText;
    QString mapWidthText;
    QString mapUnitText;
    QString mapScale1xText;
    QString mapScale2xText;
    QString mapScale3xText;
    QString mapScale4xText;
    bool escToClose = false;
    bool altRightMouseSurvey = false;
    bool spaceCenterSurvey = false;
    bool showCrosshair = false;
};

inline UiSettingsDraft defaultUiSettingsDraft() {
    UiSettingsDraft draft;
    draft.toggleFeature.text = QStringLiteral("M");
    draft.toggleFeatureAlt.text = QString();
    draft.markPoint.text = QStringLiteral("Ctrl");
    draft.pointSizeText = QStringLiteral("5");
    draft.lineThicknessText = QStringLiteral("2");
    draft.mapWidthText = QStringLiteral("80");
    draft.mapUnitText = QStringLiteral("100");
    draft.mapScale1xText = QStringLiteral("2.10526");
    draft.mapScale2xText = QStringLiteral("4");
    draft.mapScale3xText = QStringLiteral("8");
    draft.mapScale4xText = QStringLiteral("16");
    draft.escToClose = true;
    draft.altRightMouseSurvey = true;
    draft.spaceCenterSurvey = true;
    draft.showCrosshair = true;
    return draft;
}
