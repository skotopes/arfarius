#include "playlist.h"
#include <QMimeData>
#include <QDebug>

PlayList::PlayList(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int PlayList::rowCount(const QModelIndex & /*parent*/) const
{
   return 2;
}

int PlayList::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant PlayList::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
       return QString("Row%1, Column%2")
                   .arg(index.row() + 1)
                   .arg(index.column() +1);
    }
    return QVariant();
}
