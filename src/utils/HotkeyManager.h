#pragma once

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QKeySequence>
#include <xcb/xcb.h>

class HotkeyManager : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
public:
    explicit HotkeyManager(QObject *parent = nullptr);
    ~HotkeyManager() override;

    void registerHotkey(const QKeySequence &keys);
    void unregisterHotkey();

    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           qintptr *result) override;

signals:
    void activated();

private:
    bool m_registered = false;
    xcb_connection_t *m_connection = nullptr;
    xcb_window_t m_root = XCB_WINDOW_NONE;
    xcb_keycode_t m_keycode = 0;
    uint16_t m_modifiers = 0;
};
