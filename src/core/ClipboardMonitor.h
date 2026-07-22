#pragma once

#include <QObject>
#include <QClipboard>
#include <QTimer>
#include "ClipboardItem.h"

class ClipboardMonitor : public QObject {
    Q_OBJECT
public:
    explicit ClipboardMonitor(QObject *parent = nullptr);

    void start();
    void stop();
    void setSelfChange(bool v) { m_selfChange = v; }

public slots:
    void copyToClipboard(const ClipboardItem &item);

signals:
    void newItem(const ClipboardItem &item);

private slots:
    void onClipboardChanged();

private:
    ClipboardItem captureCurrent();

    QClipboard *m_clipboard;
    QTimer *m_debounceTimer;
    QString m_lastHash;
    bool m_selfChange = false;
};
