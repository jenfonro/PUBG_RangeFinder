#include "ui/KeyCaptureLineEdit.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMouseEvent>

KeyCaptureLineEdit::KeyCaptureLineEdit(QWidget* parent)
    : QLineEdit(parent),
      allowModifierOnly_(false) {
    setReadOnly(true);
    setMinimumWidth(220);
}

void KeyCaptureLineEdit::setBindingText(const QString& text) {
    setText(text);
    setCursorPosition(0);
}

void KeyCaptureLineEdit::setAllowModifierOnly(bool enabled) {
    allowModifierOnly_ = enabled;
}

void KeyCaptureLineEdit::mousePressEvent(QMouseEvent* event) {
    QLineEdit::mousePressEvent(event);
    selectAll();
}

void KeyCaptureLineEdit::focusInEvent(QFocusEvent* event) {
    QLineEdit::focusInEvent(event);
    selectAll();
}

void KeyCaptureLineEdit::focusOutEvent(QFocusEvent* event) {
    QLineEdit::focusOutEvent(event);
}

void KeyCaptureLineEdit::keyPressEvent(QKeyEvent* event) {
    if (event == nullptr) {
        return;
    }

    const int key = event->key();
    if (key == Qt::Key_Tab || key == Qt::Key_Backtab) {
        QLineEdit::keyPressEvent(event);
        return;
    }

    if (isModifierKey(key)) {
        if (allowModifierOnly_) {
            const QString modifierText = modifierKeyText(key);
            if (!modifierText.isEmpty()) {
                setBindingText(modifierText);
            }
        }
        event->accept();
        return;
    }

    Qt::KeyboardModifiers modifiers = event->modifiers();
    modifiers &= (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);

    const QKeySequence sequence(static_cast<int>(modifiers) | key);
    const QString bindingText = sequence.toString(QKeySequence::NativeText);
    if (!bindingText.isEmpty()) {
        setBindingText(bindingText);
    }
    event->accept();
}

bool KeyCaptureLineEdit::isModifierKey(int key) {
    return key == Qt::Key_Control ||
           key == Qt::Key_Shift ||
           key == Qt::Key_Alt ||
           key == Qt::Key_Meta;
}

QString KeyCaptureLineEdit::modifierKeyText(int key) {
    switch (key) {
    case Qt::Key_Control:
        return QStringLiteral("Ctrl");
    case Qt::Key_Shift:
        return QStringLiteral("Shift");
    case Qt::Key_Alt:
        return QStringLiteral("Alt");
    case Qt::Key_Meta:
        return QStringLiteral("Meta");
    default:
        return QString();
    }
}
