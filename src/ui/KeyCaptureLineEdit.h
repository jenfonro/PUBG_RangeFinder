#pragma once

#include <QLineEdit>

class KeyCaptureLineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit KeyCaptureLineEdit(QWidget* parent = nullptr);
    void setBindingText(const QString& text);
    void setAllowModifierOnly(bool enabled);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    static bool isModifierKey(int key);
    static QString modifierKeyText(int key);

    bool allowModifierOnly_;
};
