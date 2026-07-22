#pragma once

#include <QStyledItemDelegate>

class HistoryListDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit HistoryListDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    static QPixmap createRoundedPixmap(const QPixmap &src, int radius);
};
