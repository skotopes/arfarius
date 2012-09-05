#include "playlist.h"

#include "playlistitem.h"
#include "avexception.h"
#include "avfile.h"

#include <QMimeData>
#include <QDebug>

PlayList::PlayList(QObject *parent) :
    QAbstractTableModel(parent), items()
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
            return i->source.path();
        } else if (index.column() == 1) {
            return i->artist;
        } else if (index.column() == 2) {
            return i->name;
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
    } else {
        return QString("Row %1").arg(section);
    }
}


void PlayList::appendUrls(QList<QUrl> urls)
{
    QList<QUrl>::iterator i;
    for (i = urls.begin(); i != urls.end(); ++i) {
        // validate media
        try {
            AVFile f;
            // Prepare for ultimate combo
            f.open((*i).toString().toLocal8Bit().constData());
            // now we ready
            if (f.isAudio()) {
                PlayListItem *p = new PlayListItem(*i);
                beginInsertRows(QModelIndex(), items.count(), items.count() + 1);
                items.append(p);
                endInsertRows();
            }
        } catch (AVException &e) {
            qDebug() << "PlayList:" << e.what();
        }
    }
}
