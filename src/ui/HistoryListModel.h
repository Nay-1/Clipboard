#pragma once

#include <QAbstractListModel>
#include <QVector>
#include "core/ClipboardItem.h"

class HistoryListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        TextRole,
        HtmlRole,
        ImageRole,
        TimeRole,
        FavoriteRole,
        HashRole
    };

    explicit HistoryListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const QVector<ClipboardItem> &items);
    void prependItem(const ClipboardItem &item);
    void removeItem(qint64 id);
    void updateItem(const ClipboardItem &item);
    ClipboardItem getItem(int row) const;
    void clear();

private:
    QVector<ClipboardItem> m_items;
};
