#include "ImagePreviewWidget.h"

#include <QVBoxLayout>
#include <QPixmap>
#include <QResizeEvent>

ImagePreviewWidget::ImagePreviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_label(new QLabel(this))
    , m_scrollArea(new QScrollArea(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_label->setScaledContents(false);
    m_label->setWordWrap(true);
    m_label->setStyleSheet(
        "QLabel { padding: 12px; background: transparent; color: #555; font-size: 13px; line-height: 1.5; }"
    );
    m_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_label->setContextMenuPolicy(Qt::NoContextMenu);

    m_scrollArea->setWidget(m_label);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 4px; }"
        "QScrollBar::handle:vertical { background: #DDD; border-radius: 2px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    layout->addWidget(m_scrollArea);
}

void ImagePreviewWidget::setImage(const QByteArray &data)
{
    m_currentImageData = data;
    QPixmap pixmap;
    if (pixmap.loadFromData(data)) {
        m_originalPixmap = pixmap;
        updateScaledPixmap();
    }
}

void ImagePreviewWidget::setText(const QString &text)
{
    m_originalPixmap = QPixmap();
    m_currentImageData.clear();
    m_label->setText(text);
}

void ImagePreviewWidget::clear()
{
    m_originalPixmap = QPixmap();
    m_currentImageData.clear();
    m_label->clear();
}

void ImagePreviewWidget::updateScaledPixmap()
{
    if (m_originalPixmap.isNull()) return;
    QSize available = m_scrollArea->viewport()->size();
    QPixmap scaled = m_originalPixmap.scaled(available, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_label->setPixmap(scaled);
    m_label->resize(scaled.size());
}

void ImagePreviewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!m_originalPixmap.isNull())
        updateScaledPixmap();
}
