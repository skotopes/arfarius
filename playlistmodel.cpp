#include "playlistmodel.h"
#include "playlistitem.h"

#include <QDirIterator>
#include <QtConcurrent>
#include <QFileInfo>
#include <QMimeData>
#include <QColor>
#include <QDebug>

PlayListModel::PlayListModel(QObject *parent) :
    QAbstractTableModel(parent), items(), current(-1)
{
}

int PlayListModel::rowCount(const QModelIndex & /*parent*/) const
{
    return items.count();
}

int PlayListModel::columnCount(const QModelIndex & /*parent*/) const
{
    return PlayListItem::getColumnsCount();
}

Qt::ItemFlags PlayListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QVariant PlayListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        PlayListItem *i = items[index.row()];
        return i->getColumn(index.column());
    } else if (role == Qt::BackgroundRole && index.row() == current) {
        return QVariant(QColor(152,209,117));
    } else if (role == Qt::EditRole) {
        PlayListItem *i = items[index.row()];
        return i->getColumn(index.column());
    }

    return QVariant();
}

bool PlayListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        PlayListItem *i = items[index.row()];

        i->setColumn(index.column(), value.toString());

        emit(dataChanged(index, index));
        return true;
    }

    return false;
}

QVariant PlayListModel::headerData(int section, Qt::Orientation orientation,
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

bool PlayListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    beginRemoveRows( QModelIndex(), row, row + count - 1);

    PlayListItem *current_item = nullptr;
    if (current >= 0 && current < items.size()) {
        current_item = items[current];
    }

    for (int i = 0; i < count; ++i) {
        items.removeAt(row);
    }

    if (current_item) {
        current = items.indexOf(current_item);
        if (current == -1) {
            emit itemUpdated(nullptr);
        }
    }

    endRemoveRows();

    return true;
}

void PlayListModel::clickedItem(const QModelIndex &index)
{
    int previous = current;
    current = index.row();
    emit itemUpdated(items[current]);
    emit dataChanged(createIndex(previous, 0), createIndex(previous, PlayListItem::getColumnsCount()));
    emit dataChanged(createIndex(current, 0), createIndex(current, PlayListItem::getColumnsCount()));
}

void PlayListModel::nextItem()
{
    if (!items.count()) {
        emit itemUpdated(nullptr);
    } else {
        int previous = current;
        if (++current >= items.size()) {
            current = -1;
            emit itemUpdated(nullptr);
            emit dataChanged(createIndex(previous, 0), createIndex(previous, PlayListItem::getColumnsCount()));
        } else {
            emit itemUpdated(items[current]);
            emit dataChanged(createIndex(previous, 0), createIndex(previous, PlayListItem::getColumnsCount()));
            emit dataChanged(createIndex(current, 0), createIndex(current, PlayListItem::getColumnsCount()));
        }
    }
}

void PlayListModel::prevItem()
{
    if (!items.count()) {
        emit itemUpdated(nullptr);
    } else {
        int previous = current;
        if (--current < 0) {
            current = -1;
            emit itemUpdated(nullptr);
            emit dataChanged(createIndex(previous, 0), createIndex(previous, PlayListItem::getColumnsCount()));
        } else {
            emit itemUpdated(items[current]);
            emit dataChanged(createIndex(previous, 0), createIndex(previous, PlayListItem::getColumnsCount()));
            emit dataChanged(createIndex(current, 0), createIndex(current, PlayListItem::getColumnsCount()));
        }
    }
}

void PlayListModel::appendUrl(QUrl url)
{
    if (url.isLocalFile()) {
        appendItems(urlToItems(url));
    }
}

void PlayListModel::appendUrls(QList<QUrl> urls)
{
    QList<PlayListItem *> new_items;
    QListIterator<QUrl> urls_iterator(urls);
    while (urls_iterator.hasNext()) {
        QUrl url = urls_iterator.next();
        if (url.isLocalFile()) {
            new_items += urlToItems(url);
        }
    }

    appendItems(new_items);
}

void PlayListModel::appendItems(QList<PlayListItem *> new_items)
{
    if (new_items.length() > 0) {
        beginInsertRows(QModelIndex(), new_items.count(), new_items.count() + new_items.size()-1);
        items += new_items;
        endInsertRows();
    }
}

QList<PlayListItem *> PlayListModel::urlToItems(QUrl url)
{
    QList<PlayListItem *> new_items;

    QFileInfo f(url.path());
    if (f.isDir()) {
        QDirIterator iterator(url.path(), QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            iterator.next();
            if (!iterator.fileInfo().isDir()) {
                new_items += urlToItems(QUrl::fromLocalFile(iterator.filePath()));
            }
        }
    } else {
        PlayListItem *p = new PlayListItem(url);
        if (p->isValid()) {
            p->ensureHistogram();
            new_items += p;
        }
    }

    return new_items;
}
