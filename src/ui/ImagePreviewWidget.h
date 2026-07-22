#pragma once

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QPixmap>

class ImagePreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImagePreviewWidget(QWidget *parent = nullptr);

    void setImage(const QByteArray &data);
    void setText(const QString &text);
    void clear();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateScaledPixmap();

    QLabel *m_label;
    QScrollArea *m_scrollArea;
    QPixmap m_originalPixmap;
    QByteArray m_currentImageData;
};
