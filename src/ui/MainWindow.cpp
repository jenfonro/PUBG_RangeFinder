#include "ui/MainWindow.h"

#include "ui/KeyCaptureLineEdit.h"

#include <QCheckBox>
#include <QEvent>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      titleBar_(nullptr),
      titleTextLabel_(nullptr),
      closeButton_(nullptr),
      toggleFeatureEdit_(nullptr),
      toggleFeatureAltEdit_(nullptr),
      markPointEdit_(nullptr),
      pointSizeEdit_(nullptr),
      lineThicknessEdit_(nullptr),
      mapWidthEdit_(nullptr),
      mapUnitEdit_(nullptr),
      mapScale1xEdit_(nullptr),
      mapScale2xEdit_(nullptr),
      mapScale3xEdit_(nullptr),
      mapScale4xEdit_(nullptr),
      escToCloseCheck_(nullptr),
      altRightMouseSurveyCheck_(nullptr),
      spaceCenterSurveyCheck_(nullptr),
      showCrosshairCheck_(nullptr),
      cancelButton_(nullptr),
      applyButton_(nullptr),
      draggingWindow_(false) {
    setWindowTitle(QStringLiteral("PUBG Rangefinder"));
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    resize(820, 520);
    setMinimumSize(760, 460);
    setCentralWidget(createCentralPanel());
    applyVisualStyle();

    connect(closeButton_, &QPushButton::clicked, this, &QWidget::close);
    connect(cancelButton_, &QPushButton::clicked, this, &QWidget::close);
    connect(applyButton_, &QPushButton::clicked, this, [this]() {
        emit settingsApplied(collectDraft());
    });
}

void MainWindow::loadDraft(const UiSettingsDraft& draft) {
    toggleFeatureEdit_->setBindingText(draft.toggleFeature.text);
    toggleFeatureAltEdit_->setBindingText(draft.toggleFeatureAlt.text);
    markPointEdit_->setBindingText(draft.markPoint.text);
    pointSizeEdit_->setText(draft.pointSizeText);
    lineThicknessEdit_->setText(draft.lineThicknessText);
    mapWidthEdit_->setText(draft.mapWidthText);
    mapUnitEdit_->setText(draft.mapUnitText);
    mapScale1xEdit_->setText(draft.mapScale1xText);
    mapScale2xEdit_->setText(draft.mapScale2xText);
    mapScale3xEdit_->setText(draft.mapScale3xText);
    mapScale4xEdit_->setText(draft.mapScale4xText);
    escToCloseCheck_->setChecked(draft.escToClose);
    altRightMouseSurveyCheck_->setChecked(draft.altRightMouseSurvey);
    spaceCenterSurveyCheck_->setChecked(draft.spaceCenterSurvey);
    showCrosshairCheck_->setChecked(draft.showCrosshair);
}

UiSettingsDraft MainWindow::collectDraft() const {
    UiSettingsDraft draft;
    draft.toggleFeature.text = toggleFeatureEdit_->text();
    draft.toggleFeatureAlt.text = toggleFeatureAltEdit_->text();
    draft.markPoint.text = markPointEdit_->text();
    draft.pointSizeText = pointSizeEdit_->text();
    draft.lineThicknessText = lineThicknessEdit_->text();
    draft.mapWidthText = mapWidthEdit_->text();
    draft.mapUnitText = mapUnitEdit_->text();
    draft.mapScale1xText = mapScale1xEdit_->text();
    draft.mapScale2xText = mapScale2xEdit_->text();
    draft.mapScale3xText = mapScale3xEdit_->text();
    draft.mapScale4xText = mapScale4xEdit_->text();
    draft.escToClose = escToCloseCheck_->isChecked();
    draft.altRightMouseSurvey = altRightMouseSurveyCheck_->isChecked();
    draft.spaceCenterSurvey = spaceCenterSurveyCheck_->isChecked();
    draft.showCrosshair = showCrosshairCheck_->isChecked();
    return draft;
}

void MainWindow::showSettingsWindow() {
    show();
    raise();
    activateWindow();
}

QWidget* MainWindow::createCentralPanel() {
    QWidget* central = new QWidget(this);
    central->setObjectName(QStringLiteral("windowRoot"));
    QVBoxLayout* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(14, 14, 14, 14);
    rootLayout->setSpacing(14);

    QFrame* contentFrame = new QFrame(central);
    contentFrame->setObjectName(QStringLiteral("contentFrame"));
    QVBoxLayout* contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(12, 12, 12, 12);
    contentLayout->setSpacing(14);

    QHBoxLayout* columnLayout = new QHBoxLayout();
    columnLayout->setSpacing(16);
    columnLayout->addWidget(createHotkeyGroup(), 3);

    QVBoxLayout* rightColumnLayout = new QVBoxLayout();
    rightColumnLayout->setContentsMargins(0, 0, 0, 0);
    rightColumnLayout->setSpacing(16);
    rightColumnLayout->addWidget(createListenGroup());
    rightColumnLayout->addWidget(createMapGroup());
    rightColumnLayout->addStretch(1);

    QWidget* rightColumn = new QWidget(contentFrame);
    rightColumn->setLayout(rightColumnLayout);
    columnLayout->addWidget(rightColumn, 2);

    contentLayout->addWidget(createTitleBar());
    contentLayout->addLayout(columnLayout);
    contentLayout->addWidget(createBottomActions());
    contentLayout->addStretch(1);
    rootLayout->addWidget(contentFrame);

    return central;
}

QWidget* MainWindow::createTitleBar() {
    titleBar_ = new QFrame(this);
    titleBar_->setObjectName(QStringLiteral("titleBar"));
    titleBar_->installEventFilter(this);

    QHBoxLayout* layout = new QHBoxLayout(titleBar_);
    layout->setContentsMargins(8, 4, 4, 4);
    layout->setSpacing(8);

    titleTextLabel_ = new QLabel(QStringLiteral("PUBG 测距工具"), titleBar_);
    titleTextLabel_->setObjectName(QStringLiteral("windowTitleLabel"));
    titleTextLabel_->installEventFilter(this);

    closeButton_ = new QPushButton(QStringLiteral("×"), titleBar_);
    closeButton_->setObjectName(QStringLiteral("titleBarButtonClose"));

    layout->addWidget(titleTextLabel_);
    layout->addStretch(1);
    layout->addWidget(closeButton_);

    return titleBar_;
}

QWidget* MainWindow::createHotkeyGroup() {
    QFrame* group = new QFrame(this);
    group->setObjectName(QStringLiteral("cardFrame"));
    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    groupLayout->setContentsMargins(18, 18, 18, 18);
    groupLayout->setSpacing(12);

    QLabel* title = new QLabel(QStringLiteral("快捷设置"), group);
    title->setObjectName(QStringLiteral("groupTitleLabel"));

    QFormLayout* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignTop);
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(12);

    toggleFeatureEdit_ = new KeyCaptureLineEdit(group);
    toggleFeatureAltEdit_ = new KeyCaptureLineEdit(group);
    markPointEdit_ = new KeyCaptureLineEdit(group);
    markPointEdit_->setAllowModifierOnly(true);
    pointSizeEdit_ = new QLineEdit(group);
    lineThicknessEdit_ = new QLineEdit(group);
    pointSizeEdit_->setPlaceholderText(QStringLiteral("5"));
    pointSizeEdit_->setMinimumWidth(220);
    lineThicknessEdit_->setPlaceholderText(QStringLiteral("2"));
    lineThicknessEdit_->setMinimumWidth(220);

    form->addRow(QStringLiteral("功能开关"), toggleFeatureEdit_);
    form->addRow(QStringLiteral("备用功能开关"), toggleFeatureAltEdit_);
    form->addRow(QStringLiteral("标记点"), markPointEdit_);
    form->addRow(QStringLiteral("标记点大小"), pointSizeEdit_);
    form->addRow(QStringLiteral("标记线粗细"), lineThicknessEdit_);

    groupLayout->addWidget(title);
    groupLayout->addLayout(form);
    groupLayout->addStretch(1);

    return group;
}

QWidget* MainWindow::createListenGroup() {
    QFrame* group = new QFrame(this);
    group->setObjectName(QStringLiteral("cardFrame"));
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    QLabel* title = new QLabel(QStringLiteral("监听选项"), group);
    title->setObjectName(QStringLiteral("groupTitleLabel"));

    escToCloseCheck_ = new QCheckBox(QStringLiteral("监听ESC关闭"), group);
    altRightMouseSurveyCheck_ = new QCheckBox(QStringLiteral("监听Alt + 鼠标右键测绘（无定位线）"), group);
    spaceCenterSurveyCheck_ = new QCheckBox(QStringLiteral("监听空格 + 鼠标左键以中心点测绘"), group);
    showCrosshairCheck_ = new QCheckBox(QStringLiteral("显示准星"), group);

    layout->addWidget(title);
    layout->addWidget(escToCloseCheck_);
    layout->addWidget(altRightMouseSurveyCheck_);
    layout->addWidget(spaceCenterSurveyCheck_);
    layout->addWidget(showCrosshairCheck_);
    layout->addStretch(1);

    return group;
}

QWidget* MainWindow::createMapGroup() {
    QFrame* group = new QFrame(this);
    group->setObjectName(QStringLiteral("cardFrame"));
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    QLabel* title = new QLabel(QStringLiteral("地图选项"), group);
    title->setObjectName(QStringLiteral("groupTitleLabel"));

    QGridLayout* grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);

    mapWidthEdit_ = new QLineEdit(group);
    mapUnitEdit_ = new QLineEdit(group);
    mapScale1xEdit_ = new QLineEdit(group);
    mapScale2xEdit_ = new QLineEdit(group);
    mapScale3xEdit_ = new QLineEdit(group);
    mapScale4xEdit_ = new QLineEdit(group);

    mapWidthEdit_->setObjectName(QStringLiteral("compactMapLineEdit"));
    mapUnitEdit_->setObjectName(QStringLiteral("compactMapLineEdit"));
    mapScale1xEdit_->setObjectName(QStringLiteral("compactMapLineEdit"));
    mapScale2xEdit_->setObjectName(QStringLiteral("compactMapLineEdit"));
    mapScale3xEdit_->setObjectName(QStringLiteral("compactMapLineEdit"));
    mapScale4xEdit_->setObjectName(QStringLiteral("compactMapLineEdit"));

    mapWidthEdit_->setMinimumWidth(92);
    mapUnitEdit_->setMinimumWidth(92);
    mapScale1xEdit_->setMinimumWidth(92);
    mapScale2xEdit_->setMinimumWidth(92);
    mapScale3xEdit_->setMinimumWidth(92);
    mapScale4xEdit_->setMinimumWidth(92);
    mapWidthEdit_->setMaximumWidth(120);
    mapUnitEdit_->setMaximumWidth(120);
    mapScale1xEdit_->setMaximumWidth(120);
    mapScale2xEdit_->setMaximumWidth(120);
    mapScale3xEdit_->setMaximumWidth(120);
    mapScale4xEdit_->setMaximumWidth(120);
    mapWidthEdit_->setPlaceholderText(QStringLiteral("80"));
    mapUnitEdit_->setPlaceholderText(QStringLiteral("100"));
    mapScale1xEdit_->setPlaceholderText(QStringLiteral("2.10526"));
    mapScale2xEdit_->setPlaceholderText(QStringLiteral("4"));
    mapScale3xEdit_->setPlaceholderText(QStringLiteral("8"));
    mapScale4xEdit_->setPlaceholderText(QStringLiteral("16"));

    QLabel* mapUnitLabel = new QLabel(QStringLiteral("地图单位"), group);
    QLabel* mapWidthLabel = new QLabel(QStringLiteral("地图宽度"), group);
    QLabel* scale1xLabel = new QLabel(QStringLiteral("1x"), group);
    QLabel* scale2xLabel = new QLabel(QStringLiteral("2x"), group);
    QLabel* scale3xLabel = new QLabel(QStringLiteral("3x"), group);
    QLabel* scale4xLabel = new QLabel(QStringLiteral("4x"), group);

    grid->addWidget(mapUnitLabel, 0, 0);
    grid->addWidget(mapUnitEdit_, 0, 1);
    grid->addWidget(mapWidthLabel, 0, 2);
    grid->addWidget(mapWidthEdit_, 0, 3);

    grid->addWidget(scale1xLabel, 1, 0);
    grid->addWidget(mapScale1xEdit_, 1, 1);
    grid->addWidget(scale2xLabel, 1, 2);
    grid->addWidget(mapScale2xEdit_, 1, 3);

    grid->addWidget(scale3xLabel, 2, 0);
    grid->addWidget(mapScale3xEdit_, 2, 1);
    grid->addWidget(scale4xLabel, 2, 2);
    grid->addWidget(mapScale4xEdit_, 2, 3);

    grid->setColumnStretch(1, 0);
    grid->setColumnStretch(3, 0);

    layout->addWidget(title);
    layout->addLayout(grid);

    return group;
}

QWidget* MainWindow::createBottomActions() {
    QWidget* container = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 2, 0, 0);
    layout->setSpacing(12);

    cancelButton_ = new QPushButton(QStringLiteral("取消"), container);
    applyButton_ = new QPushButton(QStringLiteral("应用"), container);
    applyButton_->setDefault(true);

    layout->addStretch(1);
    layout->addWidget(cancelButton_);
    layout->addWidget(applyButton_);

    return container;
}

void MainWindow::applyVisualStyle() {
    setStyleSheet(QStringLiteral(R"(
        QMainWindow {
            background: transparent;
        }
        QWidget#windowRoot {
            background: transparent;
        }
        QFrame#contentFrame {
            background: #f7f9fb;
            border: none;
            border-radius: 18px;
        }
        QFrame#titleBar {
            background: transparent;
            border: none;
        }
        QLabel#windowTitleLabel {
            color: #163047;
            font-size: 23px;
            font-weight: 700;
        }
        QLabel#groupTitleLabel {
            color: #17324d;
            font-size: 16px;
            font-weight: 700;
        }
        QFrame#cardFrame {
            background: #ffffff;
            border: 1px solid #d6e0e7;
            border-radius: 14px;
        }
        QLineEdit {
            background: #fbfcfd;
            border: 1px solid #cfd8df;
            border-radius: 8px;
            padding: 8px 10px;
            min-height: 18px;
        }
        QLineEdit#compactMapLineEdit {
            padding: 6px 8px;
            min-height: 16px;
        }
        QLineEdit:focus {
            border: 1px solid #4a7da8;
        }
        QCheckBox {
            color: #22394f;
            spacing: 10px;
        }
        QPushButton {
            background: #eaf1f6;
            border: 1px solid #c7d7e2;
            border-radius: 8px;
            color: #1b364e;
            min-width: 92px;
            min-height: 34px;
            padding: 0 12px;
        }
        QPushButton:hover {
            background: #dfeaf2;
        }
        QPushButton:default {
            background: #2f6c99;
            border: 1px solid #27587d;
            color: #ffffff;
        }
        QPushButton#titleBarButton, QPushButton#titleBarButtonClose {
            min-width: 34px;
            max-width: 34px;
            min-height: 30px;
            max-height: 30px;
            padding: 0;
            font-size: 16px;
            font-weight: 600;
            border-radius: 8px;
        }
        QPushButton#titleBarButton {
            background: #eef4f8;
            border: 1px solid #d5e0e7;
            color: #23425e;
        }
        QPushButton#titleBarButtonClose {
            background: #fff1f0;
            border: 1px solid #f0c5c2;
            color: #962e2b;
        }
        QPushButton#titleBarButton:hover {
            background: #e0ebf2;
        }
        QPushButton#titleBarButtonClose:hover {
            background: #fbdedd;
        }
    )"));
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if ((watched == titleBar_ || watched == titleTextLabel_) && event != nullptr) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                draggingWindow_ = true;
                dragOffset_ = mouseEvent->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
            break;
        }
        case QEvent::MouseMove: {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (draggingWindow_ && (mouseEvent->buttons() & Qt::LeftButton)) {
                move(mouseEvent->globalPosition().toPoint() - dragOffset_);
                return true;
            }
            break;
        }
        case QEvent::MouseButtonRelease:
            draggingWindow_ = false;
            break;
        default:
            break;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}
