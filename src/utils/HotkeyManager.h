#pragma once

#include <QObject>
#include <QShortcut>

class HotkeyManager : public QObject {
    Q_OBJECT
public:
    explicit HotkeyManager(QObject *parent = nullptr);

    void registerHotkey(const QKeySequence &keys);
    void unregisterHotkey();

signals:
    void activated();

private:
    QShortcut *m_shortcut = nullptr;
};
