#include "DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDateTime>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
}

bool DatabaseManager::initialize()
{
    QString dbPath = QDir::currentPath() + "/clipboard_history.db";

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    if (!createTable())
        return false;

    cleanupOldItems();
    return true;
}

bool DatabaseManager::createTable()
{
    QSqlQuery query(m_db);
    return query.exec(
        "CREATE TABLE IF NOT EXISTS clipboard_history ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  type INTEGER NOT NULL DEFAULT 0,"
        "  content_text TEXT,"
        "  content_html TEXT,"
        "  image_data BLOB,"
        "  content_hash TEXT UNIQUE,"
        "  is_favorite INTEGER NOT NULL DEFAULT 0,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")"
    );
}

qint64 DatabaseManager::addItem(const ClipboardItem &item)
{
    if (itemExists(item.contentHash))
        return 0;

    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO clipboard_history "
        "(type, content_text, content_html, image_data, content_hash, is_favorite, created_at) "
        "VALUES (:type, :text, :html, :image, :hash, :fav, :time)"
    );
    query.bindValue(":type", static_cast<int>(item.type));
    query.bindValue(":text", item.contentText);
    query.bindValue(":html", item.contentHtml);
    query.bindValue(":image", item.imageData);
    query.bindValue(":hash", item.contentHash);
    query.bindValue(":fav", item.isFavorite ? 1 : 0);
    query.bindValue(":time", item.createdAt.toString(Qt::ISODate));

    if (!query.exec()) {
        qWarning() << "Failed to add item:" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toLongLong();
}

bool DatabaseManager::removeItem(qint64 id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM clipboard_history WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}

bool DatabaseManager::toggleFavorite(qint64 id)
{
    ClipboardItem item = getItem(id);
    if (item.id < 0) return false;

    bool wasFav = item.isFavorite;
    QSqlQuery query(m_db);
    if (wasFav) {
        query.prepare("UPDATE clipboard_history SET is_favorite = 0, created_at = :time WHERE id = :id");
        query.bindValue(":time", QDateTime::currentDateTime().toString(Qt::ISODate));
    } else {
        query.prepare("UPDATE clipboard_history SET is_favorite = 1 WHERE id = :id");
    }
    query.bindValue(":id", id);
    return query.exec();
}

bool DatabaseManager::clearAll()
{
    QSqlQuery query(m_db);
    return query.exec("DELETE FROM clipboard_history");
}

QVector<ClipboardItem> DatabaseManager::getAllItems(int limit)
{
    QVector<ClipboardItem> items;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT * FROM clipboard_history "
        "ORDER BY is_favorite DESC, created_at DESC LIMIT :limit"
    );
    query.bindValue(":limit", limit);

    if (!query.exec())
        return items;

    while (query.next())
        items.append(rowToItem(query));

    return items;
}

QVector<ClipboardItem> DatabaseManager::searchItems(const QString &keyword, int limit)
{
    QVector<ClipboardItem> items;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT * FROM clipboard_history "
        "WHERE content_text LIKE :keyword OR content_html LIKE :keyword "
        "ORDER BY is_favorite DESC, created_at DESC LIMIT :limit"
    );
    query.bindValue(":keyword", "%" + keyword + "%");
    query.bindValue(":limit", limit);

    if (!query.exec())
        return items;

    while (query.next())
        items.append(rowToItem(query));

    return items;
}

QVector<ClipboardItem> DatabaseManager::getFavoriteItems(int limit)
{
    QVector<ClipboardItem> items;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT * FROM clipboard_history WHERE is_favorite = 1 "
        "ORDER BY created_at DESC LIMIT :limit"
    );
    query.bindValue(":limit", limit);

    if (!query.exec())
        return items;

    while (query.next())
        items.append(rowToItem(query));

    return items;
}

ClipboardItem DatabaseManager::getItem(qint64 id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM clipboard_history WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next())
        return rowToItem(query);

    return {};
}

bool DatabaseManager::itemExists(const QString &hash)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM clipboard_history WHERE content_hash = :hash");
    query.bindValue(":hash", hash);
    if (query.exec() && query.next())
        return query.value(0).toInt() > 0;
    return false;
}

int DatabaseManager::itemCount()
{
    QSqlQuery query(m_db);
    if (query.exec("SELECT COUNT(*) FROM clipboard_history") && query.next())
        return query.value(0).toInt();
    return 0;
}

int DatabaseManager::cleanupOldItems()
{
    QSqlQuery query(m_db);
    query.prepare(
        "DELETE FROM clipboard_history "
        "WHERE is_favorite = 0 "
        "AND created_at < strftime('%Y-%m-%dT%H:%M:%S', 'now', 'localtime', '-1 day')"
    );
    if (query.exec())
        return query.numRowsAffected();
    return 0;
}

ClipboardItem DatabaseManager::rowToItem(const QSqlQuery &query) const
{
    ClipboardItem item;
    item.id = query.value("id").toLongLong();
    item.type = static_cast<ClipboardItem::Type>(query.value("type").toInt());
    item.contentText = query.value("content_text").toString();
    item.contentHtml = query.value("content_html").toString();
    item.imageData = query.value("image_data").toByteArray();
    item.contentHash = query.value("content_hash").toString();
    item.isFavorite = query.value("is_favorite").toBool();
    item.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    return item;
}
