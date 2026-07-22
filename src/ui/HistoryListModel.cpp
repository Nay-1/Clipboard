#include "HistoryListModel.h"

HistoryListModel::HistoryListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int HistoryListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant HistoryListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const auto &item = m_items[index.row()];

    switch (role) {
    case IdRole:       return item.id;
    case TypeRole:     return static_cast<int>(item.type);
    case TextRole:     return item.contentText;
    case HtmlRole:     return item.contentHtml;
    case ImageRole:    return item.imageData;
    case TimeRole:     return item.createdAt;
    case FavoriteRole: return item.isFavorite;
    case HashRole:     return item.contentHash;
    case Qt::DisplayRole: return item.contentText;
    default:           return {};
    }
}

QHash<int, QByteArray> HistoryListModel::roleNames() const
{
    return {
        {IdRole, "itemId"},
        {TypeRole, "type"},
        {TextRole, "text"},
        {HtmlRole, "html"},
        {ImageRole, "image"},
        {TimeRole, "time"},
        {FavoriteRole, "favorite"},
        {HashRole, "hash"}
    };
}

void HistoryListModel::setItems(const QVector<ClipboardItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

void HistoryListModel::prependItem(const ClipboardItem &item)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_items.prepend(item);
    endInsertRows();
}

void HistoryListModel::removeItem(qint64 id)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_items.removeAt(i);
            endRemoveRows();
            return;
        }
    }
}

void HistoryListModel::updateItem(const ClipboardItem &item)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == item.id) {
            m_items[i] = item;
            emit dataChanged(index(i), index(i));
            return;
        }
    }
}

ClipboardItem HistoryListModel::getItem(int row) const
{
    if (row >= 0 && row < m_items.size())
        return m_items[row];
    return {};
}

void HistoryListModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}
