#include "platform/win/GlobalHotkeyManager.h"

#include <QStringList>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
GlobalHotkeyManager* g_hotkeyManagerInstance = nullptr;

bool parseSingleKey(const QString& token, unsigned int& virtualKey) {
    const QString upper = token.trimmed().toUpper();
    if (upper.isEmpty()) {
        return false;
    }

    if (upper.size() == 1) {
        const QChar ch = upper.at(0);
        if (ch.isLetter() || ch.isDigit()) {
            virtualKey = static_cast<unsigned int>(ch.unicode());
            return true;
        }
    }

    if (upper.startsWith(QStringLiteral("F"))) {
        bool ok = false;
        const int index = upper.mid(1).toInt(&ok);
        if (ok && index >= 1 && index <= 24) {
            virtualKey = static_cast<unsigned int>(VK_F1 + index - 1);
            return true;
        }
    }

    if (upper == QStringLiteral("ESC") || upper == QStringLiteral("ESCAPE")) {
        virtualKey = VK_ESCAPE;
        return true;
    }
    if (upper == QStringLiteral("SPACE")) {
        virtualKey = VK_SPACE;
        return true;
    }
    if (upper == QStringLiteral("TAB")) {
        virtualKey = VK_TAB;
        return true;
    }
    if (upper == QStringLiteral("ENTER") || upper == QStringLiteral("RETURN")) {
        virtualKey = VK_RETURN;
        return true;
    }
    if (upper == QStringLiteral("DELETE") || upper == QStringLiteral("DEL")) {
        virtualKey = VK_DELETE;
        return true;
    }
    if (upper == QStringLiteral("INSERT") || upper == QStringLiteral("INS")) {
        virtualKey = VK_INSERT;
        return true;
    }
    if (upper == QStringLiteral("HOME")) {
        virtualKey = VK_HOME;
        return true;
    }
    if (upper == QStringLiteral("END")) {
        virtualKey = VK_END;
        return true;
    }
    if (upper == QStringLiteral("PGUP") || upper == QStringLiteral("PAGEUP")) {
        virtualKey = VK_PRIOR;
        return true;
    }
    if (upper == QStringLiteral("PGDN") || upper == QStringLiteral("PAGEDOWN")) {
        virtualKey = VK_NEXT;
        return true;
    }

    return false;
}
}  // namespace

GlobalHotkeyManager::GlobalHotkeyManager(QObject* parent)
    : QObject(parent),
      togglePrimaryModifiers_(0),
      togglePrimaryVirtualKey_(0),
      toggleSecondaryModifiers_(0),
      toggleSecondaryVirtualKey_(0),
      markModifiers_(0),
      markVirtualKey_(0),
      togglePrimaryConfigured_(false),
      toggleSecondaryConfigured_(false),
      markBindingConfigured_(false),
      togglePrimaryActive_(false),
      toggleSecondaryActive_(false),
      markBindingActive_(false),
      escCloseEnabled_(false),
      escActive_(false),
      altRightMouseSurveyEnabled_(false),
      altSurveyActive_(false),
      spaceCenterSurveyEnabled_(false)
#ifdef Q_OS_WIN
      ,
      keyboardHook_(nullptr),
      mouseHook_(nullptr)
#endif
{
#ifdef Q_OS_WIN
    g_hotkeyManagerInstance = this;
    installHooks();
#endif
}

GlobalHotkeyManager::~GlobalHotkeyManager() {
#ifdef Q_OS_WIN
    unregisterAll();
    if (g_hotkeyManagerInstance == this) {
        g_hotkeyManagerInstance = nullptr;
    }
#endif
}

void GlobalHotkeyManager::applyBindings(const QString& primaryBinding,
                                        const QString& secondaryBinding,
                                        const QString& markBinding) {
#ifdef Q_OS_WIN
    pressedKeys_.clear();
    togglePrimaryActive_ = false;
    toggleSecondaryActive_ = false;
    markBindingActive_ = false;
    altSurveyActive_ = false;

    togglePrimaryConfigured_ = parseBinding(primaryBinding, togglePrimaryModifiers_, togglePrimaryVirtualKey_, false);
    toggleSecondaryConfigured_ = parseBinding(secondaryBinding, toggleSecondaryModifiers_, toggleSecondaryVirtualKey_, false);
    markBindingConfigured_ = parseBinding(markBinding, markModifiers_, markVirtualKey_, true);
#else
    Q_UNUSED(primaryBinding);
    Q_UNUSED(secondaryBinding);
    Q_UNUSED(markBinding);
#endif
}

void GlobalHotkeyManager::setEscCloseEnabled(bool enabled) {
    escCloseEnabled_ = enabled;
    escActive_ = false;
}

void GlobalHotkeyManager::setAltRightMouseSurveyEnabled(bool enabled) {
    altRightMouseSurveyEnabled_ = enabled;
}

void GlobalHotkeyManager::setSpaceCenterSurveyEnabled(bool enabled) {
    spaceCenterSurveyEnabled_ = enabled;
}

void GlobalHotkeyManager::installHooks() {
#ifdef Q_OS_WIN
    keyboardHook_ = SetWindowsHookExW(WH_KEYBOARD_LL, &GlobalHotkeyManager::keyboardHookProc, GetModuleHandleW(nullptr), 0);
    mouseHook_ = SetWindowsHookExW(WH_MOUSE_LL, &GlobalHotkeyManager::mouseHookProc, GetModuleHandleW(nullptr), 0);
#endif
}

void GlobalHotkeyManager::unregisterAll() {
#ifdef Q_OS_WIN
    if (keyboardHook_ != nullptr) {
        UnhookWindowsHookEx(keyboardHook_);
        keyboardHook_ = nullptr;
    }
    if (mouseHook_ != nullptr) {
        UnhookWindowsHookEx(mouseHook_);
        mouseHook_ = nullptr;
    }
#endif
}

bool GlobalHotkeyManager::parseBinding(const QString& text,
                                       unsigned int& modifiers,
                                       unsigned int& virtualKey,
                                       bool allowModifierOnly) {
    modifiers = 0;
    virtualKey = 0;

    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }

    const QStringList parts = trimmed.split(QStringLiteral("+"), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return false;
    }

    for (int i = 0; i < parts.size(); ++i) {
        const QString token = parts.at(i).trimmed();
        const QString upper = token.toUpper();

        if (upper == QStringLiteral("CTRL") || upper == QStringLiteral("CONTROL")) {
            modifiers |= MOD_CONTROL;
            continue;
        }
        if (upper == QStringLiteral("ALT")) {
            modifiers |= MOD_ALT;
            continue;
        }
        if (upper == QStringLiteral("SHIFT")) {
            modifiers |= MOD_SHIFT;
            continue;
        }
        if (upper == QStringLiteral("META") || upper == QStringLiteral("WIN") || upper == QStringLiteral("WINDOWS")) {
            modifiers |= MOD_WIN;
            continue;
        }

        if (i != parts.size() - 1) {
            return false;
        }

        if (!parseSingleKey(token, virtualKey)) {
            return false;
        }
    }

    if (virtualKey != 0) {
        return true;
    }
    return allowModifierOnly && modifiers != 0;
}

bool GlobalHotkeyManager::isBindingActive(unsigned int modifiers,
                                          unsigned int virtualKey,
                                          const QSet<unsigned int>& pressedKeys) {
    if (virtualKey != 0 && !pressedKeys.contains(normalizeVirtualKey(virtualKey))) {
        return false;
    }

    if ((modifiers & MOD_CONTROL) != 0 &&
        !pressedKeys.contains(VK_LCONTROL) &&
        !pressedKeys.contains(VK_RCONTROL) &&
        !pressedKeys.contains(VK_CONTROL)) {
        return false;
    }
    if ((modifiers & MOD_SHIFT) != 0 &&
        !pressedKeys.contains(VK_LSHIFT) &&
        !pressedKeys.contains(VK_RSHIFT) &&
        !pressedKeys.contains(VK_SHIFT)) {
        return false;
    }
    if ((modifiers & MOD_ALT) != 0 &&
        !pressedKeys.contains(VK_LMENU) &&
        !pressedKeys.contains(VK_RMENU) &&
        !pressedKeys.contains(VK_MENU)) {
        return false;
    }
    if ((modifiers & MOD_WIN) != 0 &&
        !pressedKeys.contains(VK_LWIN) &&
        !pressedKeys.contains(VK_RWIN)) {
        return false;
    }

    return true;
}

unsigned int GlobalHotkeyManager::normalizeVirtualKey(unsigned int virtualKey) {
    if (virtualKey >= 'a' && virtualKey <= 'z') {
        return virtualKey - 'a' + 'A';
    }
    return virtualKey;
}

void GlobalHotkeyManager::updateBindingStates() {
    if (togglePrimaryConfigured_) {
        const bool activeNow = isBindingActive(togglePrimaryModifiers_, togglePrimaryVirtualKey_, pressedKeys_);
        if (!togglePrimaryActive_ && activeNow) {
            emit toggleRequested();
        }
        togglePrimaryActive_ = activeNow;
    } else {
        togglePrimaryActive_ = false;
    }

    if (toggleSecondaryConfigured_) {
        const bool activeNow = isBindingActive(toggleSecondaryModifiers_, toggleSecondaryVirtualKey_, pressedKeys_);
        if (!toggleSecondaryActive_ && activeNow) {
            emit toggleRequested();
        }
        toggleSecondaryActive_ = activeNow;
    } else {
        toggleSecondaryActive_ = false;
    }

    if (escCloseEnabled_) {
        const bool activeNow = pressedKeys_.contains(VK_ESCAPE);
        if (!escActive_ && activeNow) {
            emit escCloseRequested();
        }
        escActive_ = activeNow;
    } else {
        escActive_ = false;
    }

    if (!markBindingConfigured_) {
        markBindingActive_ = false;
    } else {
        const bool activeNow = isBindingActive(markModifiers_, markVirtualKey_, pressedKeys_);
        if (!markBindingActive_ && activeNow) {
            emit markBindingActivated();
        }
        if (markBindingActive_ && !activeNow) {
            emit markBindingReleased();
        }
        markBindingActive_ = activeNow;
    }

    if (altRightMouseSurveyEnabled_) {
        const bool activeNow =
            pressedKeys_.contains(VK_LMENU) ||
            pressedKeys_.contains(VK_RMENU) ||
            pressedKeys_.contains(VK_MENU);
        if (altSurveyActive_ && !activeNow) {
            emit altSurveyBindingReleased();
        }
        altSurveyActive_ = activeNow;
    } else {
        altSurveyActive_ = false;
    }
}

LRESULT CALLBACK GlobalHotkeyManager::keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
#ifdef Q_OS_WIN
    if (nCode >= 0 && g_hotkeyManagerInstance != nullptr) {
        const KBDLLHOOKSTRUCT* keyInfo = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (keyInfo != nullptr) {
            const unsigned int key = normalizeVirtualKey(static_cast<unsigned int>(keyInfo->vkCode));
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                g_hotkeyManagerInstance->pressedKeys_.insert(key);
                g_hotkeyManagerInstance->updateBindingStates();
            } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                g_hotkeyManagerInstance->pressedKeys_.remove(key);
                g_hotkeyManagerInstance->updateBindingStates();
            }
        }
    }
#else
    Q_UNUSED(nCode);
    Q_UNUSED(wParam);
    Q_UNUSED(lParam);
#endif
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK GlobalHotkeyManager::mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
#ifdef Q_OS_WIN
    if (nCode >= 0 && g_hotkeyManagerInstance != nullptr) {
        const MSLLHOOKSTRUCT* mouseInfo = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
        if (mouseInfo != nullptr) {
            if (wParam == WM_MOUSEWHEEL) {
                const int wheelDelta = GET_WHEEL_DELTA_WPARAM(mouseInfo->mouseData);
                if (wheelDelta > 0) {
                    emit g_hotkeyManagerInstance->zoomChangedByWheel(1);
                } else if (wheelDelta < 0) {
                    emit g_hotkeyManagerInstance->zoomChangedByWheel(-1);
                }
            }

            if (wParam == WM_LBUTTONDOWN) {
                const bool spaceCenterActive =
                    g_hotkeyManagerInstance->spaceCenterSurveyEnabled_ &&
                    g_hotkeyManagerInstance->pressedKeys_.contains(VK_SPACE);
                if (spaceCenterActive) {
                    emit g_hotkeyManagerInstance->centerSurveyPointRequested(QPoint(mouseInfo->pt.x, mouseInfo->pt.y));
                } else if (g_hotkeyManagerInstance->markBindingActive_) {
                    emit g_hotkeyManagerInstance->markPointRequested(QPoint(mouseInfo->pt.x, mouseInfo->pt.y));
                }
            }

            if (wParam == WM_RBUTTONDOWN &&
                g_hotkeyManagerInstance->altRightMouseSurveyEnabled_ &&
                g_hotkeyManagerInstance->altSurveyActive_) {
                emit g_hotkeyManagerInstance->altRightPointRequested(QPoint(mouseInfo->pt.x, mouseInfo->pt.y));
            }
        }
    }
#else
    Q_UNUSED(nCode);
    Q_UNUSED(wParam);
    Q_UNUSED(lParam);
#endif
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
