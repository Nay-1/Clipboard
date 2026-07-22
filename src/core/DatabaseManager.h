#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include "ClipboardItem.h"

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool initialize();
    qint64 addItem(const ClipboardItem &item);
    bool removeItem(qint64 id);
    bool toggleFavorite(qint64 id);
    bool clearAll();

    QVector<ClipboardItem> getAllItems(int limit = 500);
    QVector<ClipboardItem> searchItems(const QString &keyword, int limit = 500);
    QVector<ClipboardItem> getFavoriteItems(int limit = 500);
    ClipboardItem getItem(qint64 id);

    bool itemExists(const QString &hash);
    int itemCount();
    int cleanupOldItems();

private:
    bool createTable();
    ClipboardItem rowToItem(const QSqlQuery &query) const;

    QSqlDatabase m_db;
};
