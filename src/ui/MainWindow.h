#pragma once

#include "config/UiSettingsDraft.h"

#include <QMainWindow>
#include <QPoint>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QFrame;
class KeyCaptureLineEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    void loadDraft(const UiSettingsDraft& draft);
    UiSettingsDraft collectDraft() const;
    void showSettingsWindow();

signals:
    void settingsApplied(const UiSettingsDraft& draft);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QWidget* createCentralPanel();
    QWidget* createTitleBar();
    QWidget* createHotkeyGroup();
    QWidget* createListenGroup();
    QWidget* createMapGroup();
    QWidget* createBottomActions();
    void applyVisualStyle();

    QFrame* titleBar_;
    QLabel* titleTextLabel_;
    QPushButton* closeButton_;
    KeyCaptureLineEdit* toggleFeatureEdit_;
    KeyCaptureLineEdit* toggleFeatureAltEdit_;
    KeyCaptureLineEdit* markPointEdit_;
    QLineEdit* pointSizeEdit_;
    QLineEdit* lineThicknessEdit_;
    QLineEdit* mapWidthEdit_;
    QLineEdit* mapUnitEdit_;
    QLineEdit* mapScale1xEdit_;
    QLineEdit* mapScale2xEdit_;
    QLineEdit* mapScale3xEdit_;
    QLineEdit* mapScale4xEdit_;
    QCheckBox* escToCloseCheck_;
    QCheckBox* altRightMouseSurveyCheck_;
    QCheckBox* spaceCenterSurveyCheck_;
    QCheckBox* showCrosshairCheck_;
    QPushButton* cancelButton_;
    QPushButton* applyButton_;
    bool draggingWindow_;
    QPoint dragOffset_;
};
