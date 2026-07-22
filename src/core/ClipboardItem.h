#pragma once

#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QVector>

struct ClipboardItem {
    enum Type { Text = 0, Image = 1 };

    qint64 id = -1;
    Type type = Text;
    QString contentText;
    QString contentHtml;
    QByteArray imageData;
    QString contentHash;
    bool isFavorite = false;
    QDateTime createdAt;
};
