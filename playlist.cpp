#include "playlist.h"

#include "playlistitem.h"
#include "avexception.h"
#include "avfile.h"

#include <QMimeData>
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
    return 3;
}

QVariant PlayList::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        PlayListItem *i = items[index.row()];
        if (index.column() == 0) {
            return i->getUrl();
        } else if (index.column() == 1) {
            return i->getArtist();
        } else if (index.column() == 2) {
            return i->getName();
        }
    }

    return QVariant();
}

QVariant PlayList::headerData(int section, Qt::Orientation orientation,
                              int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal){
        if (section == 0) {
            return "Source";
        } else if (section == 1) {
            return "Artist";
        } else if (section == 2) {
            return "Name";
        }
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
    QList<QUrl>::iterator i;
    for (i = urls.begin(); i != urls.end(); ++i) {
        PlayListItem *p = new PlayListItem(*i);
        if (p->isValid()) {
            beginInsertRows(QModelIndex(), items.count(), items.count());
            items.append(p);
            endInsertRows();
        }
    }
}

bool PlayList::next()
{
    if (!items.count() || current == (items.count() - 1) )
        return false;
    current ++;
    return true;
}

bool PlayList::prev()
{
    if (current < 1)
        return false;
    current --;
    return true;
}

