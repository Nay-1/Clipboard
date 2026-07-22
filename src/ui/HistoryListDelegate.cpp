#include "HistoryListDelegate.h"
#include "HistoryListModel.h"
#include "core/ClipboardItem.h"

#include <QPainter>
#include <QPainterPath>
#include <QApplication>

static QColor SELECTED_BG("#E8F0FE");
static QColor SELECTED_BORDER("#D0DDF5");
static QColor HOVER_BG("#F5F9FF");
static QColor NORMAL_BG("#FFFFFF");
static QColor TEXT_COLOR("#333333");
static QColor SECONDARY_COLOR("#999999");
static QColor FAV_COLOR("#FFB800");
static QColor BORDER_COLOR("#F0F0F0");

HistoryListDelegate::HistoryListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void HistoryListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    bool isFav = index.data(HistoryListModel::FavoriteRole).toBool();
    int type = index.data(HistoryListModel::TypeRole).toInt();
    QString text = index.data(HistoryListModel::TextRole).toString();
    QByteArray imgData = index.data(HistoryListModel::ImageRole).toByteArray();
    QDateTime time = index.data(HistoryListModel::TimeRole).toDateTime();

    QRect r = option.rect;
    bool selected = option.state & QStyle::State_Selected;
    bool hovered = option.state & QStyle::State_MouseOver;

    QColor bg = NORMAL_BG;
    if (selected) bg = SELECTED_BG;
    else if (hovered) bg = HOVER_BG;

    QPainterPath path;
    path.addRoundedRect(QRectF(r.adjusted(4, 2, -4, -2)), 6, 6);
    painter->fillPath(path, bg);

    if (selected) {
        painter->setPen(QPen(SELECTED_BORDER, 1));
        painter->drawPath(path);
    }

    const int m = 10;
    const int iconSize = 40;
    int x = r.x() + m;
    int y = r.y() + m;
    int h = r.height() - m * 2;

    QRect iconRect(x, y + (h - iconSize) / 2, iconSize, iconSize);
    painter->setPen(Qt::NoPen);

    if (type == ClipboardItem::Image && !imgData.isEmpty()) {
        QPixmap pix;
        pix.loadFromData(imgData);
        QPixmap thumb = pix.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmap rounded = createRoundedPixmap(thumb, 8);
        painter->drawPixmap(iconRect, rounded);
    } else {
        QRectF bgRect(iconRect);
        painter->setBrush(isFav ? QColor("#FFF3E0") : QColor("#F0F0F0"));
        painter->drawRoundedRect(bgRect, 8, 8);

        QFont f = option.font;
        f.setPixelSize(16);
        f.setBold(true);
        painter->setFont(f);
        painter->setPen(isFav ? FAV_COLOR : SECONDARY_COLOR);
        painter->drawText(iconRect, Qt::AlignCenter, text.isEmpty() ? "Aa" : text.left(2).toUpper());
    }

    int tx = iconRect.right() + 10;
    int tw = r.right() - m - 4 - tx;
    if (isFav) tw -= 24;
    tw = qMax(tw, 20);

    QFont tf = option.font;
    tf.setPixelSize(13);
    painter->setFont(tf);
    QFontMetrics fm(tf);

    int textTop = r.top() + 12;
    int textH = r.height() - 28;

    if (type == ClipboardItem::Text) {
        QString display = text.simplified();
        display.replace('\n', ' ');
        QString truncated = fm.elidedText(display, Qt::ElideRight, tw);
        painter->setPen(TEXT_COLOR);
        painter->drawText(QRect(tx, textTop, tw, fm.height()), Qt::AlignLeft | Qt::AlignVCenter, truncated);

        if (textH > fm.height() * 2 + 4) {
            QString more = display.mid(fm.elidedText(display, Qt::ElideRight, tw).length()).simplified();
            if (!more.isEmpty()) {
                more = fm.elidedText(more, Qt::ElideRight, tw);
                painter->setPen(SECONDARY_COLOR);
                painter->drawText(QRect(tx, textTop + fm.height() + 2, tw, fm.height()),
                                  Qt::AlignLeft | Qt::AlignVCenter, more);
            }
        }
    } else {
        QString desc = text.isEmpty() ? QStringLiteral("[图片]") : text;
        QString truncated = fm.elidedText(desc, Qt::ElideRight, tw);
        painter->setPen(TEXT_COLOR);
        painter->drawText(QRect(tx, textTop + 4, tw, fm.height()),
                          Qt::AlignLeft | Qt::AlignVCenter, truncated);
    }

    QFont sf = tf;
    sf.setPixelSize(11);
    painter->setFont(sf);
    int by = r.bottom() - 14;
    QString timeStr = time.toString("MM-dd HH:mm");
    painter->setPen(QColor("#CCCCCC"));
    painter->drawText(QRect(tx, by, tw, 14), Qt::AlignLeft | Qt::AlignVCenter, timeStr);

    if (isFav) {
        QFont star = tf;
        star.setPixelSize(14);
        painter->setFont(star);
        painter->setPen(FAV_COLOR);
        painter->drawText(QRect(r.right() - m - 4 - 20, r.top() + (r.height() - 20) / 2, 20, 20),
                          Qt::AlignCenter, QStringLiteral("\u2605"));
    }

    painter->restore();
}

QSize HistoryListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QSize(option.rect.width(), 66);
}

QPixmap HistoryListDelegate::createRoundedPixmap(const QPixmap &src, int radius)
{
    if (src.isNull()) return src;

    QPixmap rounded(src.size());
    rounded.fill(Qt::transparent);

    QPainter p(&rounded);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QBrush(src));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(src.rect(), radius, radius);
    p.end();

    return rounded;
}

QString HistoryListDelegate::truncateText(const QString &text, int maxLines,
                                           const QFontMetrics &fm, int width)
{
    QString result;
    int lines = 0;
    QString remaining = text;

    while (!remaining.isEmpty() && lines < maxLines) {
        QString elided = fm.elidedText(remaining, Qt::ElideRight, width);
        if (!result.isEmpty()) result += '\n';
        result += elided;
        if (elided != remaining) break;

        int nl = remaining.indexOf('\n');
        if (nl >= 0)
            remaining = remaining.mid(nl + 1);
        else
            remaining.clear();

        lines++;
    }
    return result;
}
