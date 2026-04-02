#pragma once

#include <QObject>
#include <QPoint>
#include <QSet>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class GlobalHotkeyManager : public QObject {
    Q_OBJECT

public:
    explicit GlobalHotkeyManager(QObject* parent = nullptr);
    ~GlobalHotkeyManager() override;

    void applyBindings(const QString& primaryBinding, const QString& secondaryBinding, const QString& markBinding);
    void setEscCloseEnabled(bool enabled);
    void setAltRightMouseSurveyEnabled(bool enabled);
    void setSpaceCenterSurveyEnabled(bool enabled);

signals:
    void toggleRequested();
    void escCloseRequested();
    void zoomChangedByWheel(int deltaSteps);
    void markBindingActivated();
    void markPointRequested(const QPoint& point);
    void altRightPointRequested(const QPoint& point);
    void centerSurveyPointRequested(const QPoint& point);
    void markBindingReleased();
    void altSurveyBindingReleased();

private:
    void installHooks();
    void unregisterAll();
    static bool parseBinding(const QString& text, unsigned int& modifiers, unsigned int& virtualKey, bool allowModifierOnly = false);
    static bool isBindingActive(unsigned int modifiers, unsigned int virtualKey, const QSet<unsigned int>& pressedKeys);
    static unsigned int normalizeVirtualKey(unsigned int virtualKey);
    void updateBindingStates();
    static LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);

    unsigned int togglePrimaryModifiers_;
    unsigned int togglePrimaryVirtualKey_;
    unsigned int toggleSecondaryModifiers_;
    unsigned int toggleSecondaryVirtualKey_;
    unsigned int markModifiers_;
    unsigned int markVirtualKey_;
    bool togglePrimaryConfigured_;
    bool toggleSecondaryConfigured_;
    bool markBindingConfigured_;
    bool togglePrimaryActive_;
    bool toggleSecondaryActive_;
    bool markBindingActive_;
    bool escCloseEnabled_;
    bool escActive_;
    bool altRightMouseSurveyEnabled_;
    bool altSurveyActive_;
    bool spaceCenterSurveyEnabled_;
    QSet<unsigned int> pressedKeys_;
#ifdef Q_OS_WIN
    HHOOK keyboardHook_;
    HHOOK mouseHook_;
#endif
};
