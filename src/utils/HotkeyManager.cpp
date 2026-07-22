#include "HotkeyManager.h"

#include <QGuiApplication>
#include <QtGui/qguiapplication_platform.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

HotkeyManager::HotkeyManager(QObject *parent)
    : QObject(parent)
{
    QGuiApplication::instance()->installNativeEventFilter(this);
}

HotkeyManager::~HotkeyManager()
{
    unregisterHotkey();
    QGuiApplication::instance()->removeNativeEventFilter(this);
}

void HotkeyManager::registerHotkey(const QKeySequence &keys)
{
    unregisterHotkey();

    auto *x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!x11App)
        return;

    xcb_connection_t *conn = x11App->connection();
    const xcb_setup_t *setup = xcb_get_setup(conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_window_t root = iter.data->root;

    int seq = keys.isEmpty() ? 0 : keys[0].toCombined();
    Qt::KeyboardModifiers mods = Qt::KeyboardModifiers(seq & Qt::KeyboardModifierMask);
    int qtKey = seq & ~Qt::KeyboardModifierMask;

    uint16_t xMods = 0;
    if (mods & Qt::ShiftModifier) xMods |= XCB_MOD_MASK_SHIFT;
    if (mods & Qt::ControlModifier) xMods |= XCB_MOD_MASK_CONTROL;
    if (mods & Qt::AltModifier) xMods |= XCB_MOD_MASK_1;
    if (mods & Qt::MetaModifier) xMods |= XCB_MOD_MASK_4;

    Display *tmp = XOpenDisplay(nullptr);
    if (!tmp)
        return;

    QByteArray keyName;
    if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
        keyName = QByteArray(1, 'a' + (qtKey - Qt::Key_A));
    else if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)
        keyName = QByteArray(1, '0' + (qtKey - Qt::Key_0));
    else
        keyName = QKeySequence(qtKey).toString().toLower().toUtf8();

    KeySym keysym = XStringToKeysym(keyName.constData());
    xcb_keycode_t keycode = XKeysymToKeycode(tmp, keysym);
    XCloseDisplay(tmp);

    if (keycode == 0)
        return;

    xcb_void_cookie_t cookie = xcb_grab_key_checked(conn, 1, root, xMods,
                                                    keycode,
                                                    XCB_GRAB_MODE_ASYNC,
                                                    XCB_GRAB_MODE_ASYNC);
    xcb_generic_error_t *err = xcb_request_check(conn, cookie);
    if (err) {
        free(err);
        return;
    }
    xcb_flush(conn);

    m_connection = conn;
    m_root = root;
    m_keycode = keycode;
    m_modifiers = xMods;
    m_registered = true;
}

void HotkeyManager::unregisterHotkey()
{
    if (!m_registered)
        return;
    xcb_ungrab_key(m_connection, m_keycode, m_root, m_modifiers);
    xcb_flush(m_connection);
    m_registered = false;
    m_connection = nullptr;
    m_root = XCB_WINDOW_NONE;
    m_keycode = 0;
    m_modifiers = 0;
}

bool HotkeyManager::nativeEventFilter(const QByteArray &eventType, void *message,
                                       qintptr *result)
{
    Q_UNUSED(result)
    if (eventType != "xcb_generic_event_t")
        return false;

    auto *event = static_cast<xcb_generic_event_t *>(message);
    if ((event->response_type & 0x7f) != XCB_KEY_PRESS)
        return false;

    auto *keyEvent = reinterpret_cast<xcb_key_press_event_t *>(event);
    if (keyEvent->detail == m_keycode &&
        (keyEvent->state & m_modifiers) == m_modifiers &&
        (keyEvent->state & ~(XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_CONTROL |
                             XCB_MOD_MASK_1 | XCB_MOD_MASK_4)) == 0) {
        emit activated();
        return true;
    }
    return false;
}
