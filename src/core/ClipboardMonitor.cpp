#include "ClipboardMonitor.h"

#include <QGuiApplication>
#include <QMimeData>
#include <QCryptographicHash>
#include <QImage>
#include <QBuffer>

ClipboardMonitor::ClipboardMonitor(QObject *parent)
    : QObject(parent)
    , m_clipboard(QGuiApplication::clipboard())
    , m_debounceTimer(new QTimer(this))
{
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(200);
    connect(m_debounceTimer, &QTimer::timeout, this, &ClipboardMonitor::onClipboardChanged);
}

void ClipboardMonitor::start()
{
    connect(m_clipboard, &QClipboard::dataChanged, this, [this]() {
        if (!m_selfChange)
            m_debounceTimer->start();
    });
}

void ClipboardMonitor::stop()
{
    disconnect(m_clipboard, &QClipboard::dataChanged, nullptr, nullptr);
    m_debounceTimer->stop();
}

ClipboardItem ClipboardMonitor::captureCurrent()
{
    ClipboardItem item;
    const QMimeData *mimeData = m_clipboard->mimeData(QClipboard::Clipboard);

    if (mimeData->hasText()) {
        item.type = ClipboardItem::Text;
        item.contentText = mimeData->text();
        item.contentHtml = mimeData->html();
    }

    if (mimeData->hasImage()) {
        QImage img = m_clipboard->image();
        if (!img.isNull()) {
            item.type = ClipboardItem::Image;

            QByteArray imgData;
            QBuffer buffer(&imgData);
            buffer.open(QIODevice::WriteOnly);
            img.save(&buffer, "PNG");
            buffer.close();
            item.imageData = imgData;

            if (item.contentText.isEmpty())
                item.contentText = QString("[图片] %1x%2").arg(img.width()).arg(img.height());
        }
    }

    QByteArray hashData;
    if (!item.contentText.isEmpty())
        hashData += item.contentText.toUtf8();
    if (!item.imageData.isEmpty())
        hashData += item.imageData;
    item.contentHash = QCryptographicHash::hash(hashData, QCryptographicHash::Md5).toHex();

    item.createdAt = QDateTime::currentDateTime();

    return item;
}

void ClipboardMonitor::onClipboardChanged()
{
    ClipboardItem item = captureCurrent();
    if (item.contentHash.isEmpty() || item.contentHash == m_lastHash)
        return;

    m_lastHash = item.contentHash;
    emit newItem(item);
}

void ClipboardMonitor::copyToClipboard(const ClipboardItem &item)
{
    m_selfChange = true;

    if (item.type == ClipboardItem::Text) {
        m_clipboard->setText(item.contentText);
    } else if (item.type == ClipboardItem::Image) {
        QImage img;
        img.loadFromData(item.imageData);
        m_clipboard->setImage(img);
    }

    QTimer::singleShot(500, this, [this]() {
        m_selfChange = false;
    });
}
