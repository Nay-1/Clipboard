#include "HotkeyManager.h"

HotkeyManager::HotkeyManager(QObject *parent)
    : QObject(parent)
{
}

void HotkeyManager::registerHotkey(const QKeySequence &keys)
{
    unregisterHotkey();
    m_shortcut = new QShortcut(keys, parent());
    m_shortcut->setContext(Qt::ApplicationShortcut);
    connect(m_shortcut, &QShortcut::activated, this, &HotkeyManager::activated);
}

void HotkeyManager::unregisterHotkey()
{
    if (m_shortcut) {
        delete m_shortcut;
        m_shortcut = nullptr;
    }
}
