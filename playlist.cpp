#include "playlist.h"

#include "playlistitem.h"

#include <QMimeData>
#include <QColor>
#include <QDebug>

PlayList::PlayList(QObject *parent) :
    QAbstractTableModel(parent), items(), current(-1)
{
}

int PlayList::rowCount(const QModelIndex & /*parent*/) const
{
    return items.count();
}

int PlayList::columnCount(const QModelIndex & /*parent*/) const
{
    return PlayListItem::getColumnsCount();
}

QVariant PlayList::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        PlayListItem *i = items[index.row()];
        return i->getColumn(index.column());
    } else if (role == Qt::BackgroundRole && index.row() == current) {
        return QVariant(QColor(Qt::green));
    }

    return QVariant();
}

QVariant PlayList::headerData(int section, Qt::Orientation orientation,
                              int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal){
        return PlayListItem::getColumnName(section);
    } else if (orientation == Qt::Vertical) {
        return QString("Row %1").arg(section);
    }

    return QVariant();
}

PlayListItem * PlayList::getCurrent()
{
    if (current < 0)
        return 0;
    return items[current];
}

void PlayList::appendUrls(QList<QUrl> urls)
{
    int pos = items.count();
    QList<QUrl>::iterator i;
    for (i = urls.begin(); i != urls.end(); ++i) {
        PlayListItem *p = new PlayListItem(*i);
        if (p->isValid()) {
            pos++;
            p->setPos(pos);
            beginInsertRows(QModelIndex(), items.count(), items.count());
            items.append(p);
            endInsertRows();
        }
    }
}

bool PlayList::next()
{
    if (!items.count())
        return false;

    int c = current;
    if (current == (items.count() - 1)) {
        current = -1;
        emit dataChanged(createIndex(c, 0), createIndex(c,2));
        return false;
    } else {
        current ++;
        emit dataChanged(createIndex(c,0), createIndex(c,2));
        emit dataChanged(createIndex(current,0), createIndex(current,2));
        return true;
    }
}

bool PlayList::prev()
{
    if (current < 0 || !items.count())
        return false;

    int c = current;
    if (current < 1) {
        current = -1;
        emit dataChanged(createIndex(c,0), createIndex(c,2));
        return false;
    } else {
        current --;
        emit dataChanged(createIndex(c,0), createIndex(c,2));
        emit dataChanged(createIndex(current,0), createIndex(current,2));
        return true;
    }
}

