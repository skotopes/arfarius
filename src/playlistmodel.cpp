#include "playlistmodel.h"
#include "playlistitem.h"
#include "arfariusapplication.h"

#include <QProgressDialog>
#include <QDirIterator>
#include <QtConcurrent>
#include <QFileInfo>
#include <QMimeData>
#include <QColor>
#include <QDebug>

PlayListModel::PlayListModel(QObject* parent)
    : QAbstractTableModel(parent)
    , items()
    , current(-1) {
}

int PlayListModel::rowCount(const QModelIndex& /*parent*/) const {
    return items.count();
}

int PlayListModel::columnCount(const QModelIndex& /*parent*/) const {
    return PlayListItem::getColumnsCount();
}

Qt::DropActions PlayListModel::supportedDropActions() const {
    return QAbstractTableModel::supportedDropActions() | Qt::MoveAction;
}

Qt::ItemFlags PlayListModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if(index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QVariant PlayListModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid()) return QVariant();

    int row = index.row();
    if(row < 0 || row >= items.size()) return QVariant();

    PlayListItem* i = items[row];
    if(!i) return QVariant();

    if(role == Qt::DisplayRole) {
        return i->getColumn(index.column());
    } else if(role == Qt::BackgroundRole && row == current) {
        return QVariant(QColor(152, 209, 117));
    } else if(role == Qt::EditRole) {
        return i->getColumn(index.column());
    }

    return QVariant();
}

bool PlayListModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(index.isValid() && role == Qt::EditRole) {
        int row = index.row();
        if(row < 0 || row >= items.size()) return false;

        PlayListItem* i = items[row];
        if(!i) return false;

        i->setColumn(index.column(), value.toString());

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QVariant PlayListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role != Qt::DisplayRole) return QVariant();

    if(orientation == Qt::Horizontal) {
        return PlayListItem::getColumnName(section);
    } else if(orientation == Qt::Vertical) {
        return QString("Row %1").arg(section);
    }

    return QVariant();
}

bool PlayListModel::removeRows(int row, int count, const QModelIndex& parent) {
    Q_UNUSED(parent)

    if(row < 0 || count <= 0 || row >= items.size()) return false;
    if(row + count > items.size()) count = items.size() - row;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    PlayListItem* current_item = nullptr;
    if(current >= 0 && current < items.size()) {
        current_item = items[current];
    }

    for(int i = row + count - 1; i >= row; i--) {
        while(items[i]->isBusy())
            arfariusApp->processEvents(QEventLoop::AllEvents);
        items[i]->deleteLater();
        items.removeAt(i);
    }

    if(current_item) {
        current = items.indexOf(current_item);
        if(current == -1) {
            if(items.count() > row) {
                current = row;
                emit itemUpdated(items[current]);
            } else {
                emit itemUpdated(nullptr);
            }
        }
    }

    endRemoveRows();

    return true;
}

QStringList PlayListModel::mimeTypes() const {
    QStringList mime_types;
    mime_types << "url";
    return mime_types;
}

void PlayListModel::clickedItem(const QModelIndex& index) {
    if(!index.isValid()) return;

    int row = index.row();
    if(row < 0 || row >= items.size()) return;

    int previous = current;
    current = row;
    emit itemUpdated(items[current]);
    if(previous >= 0 && previous < items.size()) {
        emit dataChanged(
            createIndex(previous, 0), createIndex(previous, PlayListItem::getColumnsCount() - 1));
    }
    emit dataChanged(
        createIndex(current, 0), createIndex(current, PlayListItem::getColumnsCount() - 1));
}

void PlayListModel::nextItem() {
    qDebug() << this << "nextItem()" << current;
    if(!items.count()) {
        emit itemUpdated(nullptr);
        return;
    }

    int previous = current;
    if(++current >= items.size()) {
        current = -1;
        emit itemUpdated(nullptr);
    } else {
        emit itemUpdated(items[current]);
        emit dataChanged(
            createIndex(current, 0), createIndex(current, PlayListItem::getColumnsCount() - 1));
    }

    if(previous >= 0) {
        emit dataChanged(
            createIndex(previous, 0), createIndex(previous, PlayListItem::getColumnsCount() - 1));
    }
}

void PlayListModel::prevItem() {
    qDebug() << this << "prevItem()" << current;
    if(!items.count()) {
        emit itemUpdated(nullptr);
        return;
    }

    int previous = current;
    if(--current < 0) {
        current = -1;
        emit itemUpdated(nullptr);
    } else {
        emit itemUpdated(items[current]);
        emit dataChanged(
            createIndex(current, 0), createIndex(current, PlayListItem::getColumnsCount() - 1));
    }

    if(previous >= 0) {
        emit dataChanged(
            createIndex(previous, 0), createIndex(previous, PlayListItem::getColumnsCount() - 1));
    }
}

void PlayListModel::save(QString filename) {
    qDebug() << this << "save()";
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << this << "save(): failed to open file";
        return;
    }

    for(PlayListItem* item : items) {
        if(item) {
            file.write(item->getUrlString().toUtf8() + "\n");
        }
    }
}

void PlayListModel::gather(QString path) {
    qDebug() << this << "gather()";
    QDir dir(path);
    if(!dir.exists()) {
        qWarning() << this << "gather(): directory doesn't exist";
        return;
    }

    for(PlayListItem* item : items) {
        if(!item || !item->isLocalFile()) {
            continue;
        }
        QString src = item->getUrlStringLocal();
        QFileInfo fileinfo(src);
        QString destPath = dir.filePath(fileinfo.fileName());

        // Only move if source and destination differ
        if(src != destPath) {
            dir.rename(src, destPath);
            item->setUrl(QUrl::fromLocalFile(destPath));
        }
        item->ensureHistogram();
    }
}

void PlayListModel::removeCurrent() {
    qDebug() << this << "removeCurrent()";
    if(current < 0 || current >= items.size()) return;
    removeRows(current, 1, QModelIndex());
}

void PlayListModel::appendUrl(QUrl url) {
    appendUrls(QList<QUrl>{url});
}

void PlayListModel::appendUrls(QList<QUrl> urls) {
    QProgressDialog progress("Adding files to playlist...", "Cancel", 0, urls.count());
    progress.setWindowModality(Qt::ApplicationModal);
    progress.show();

    QList<PlayListItem*> new_items;
    QListIterator<QUrl> urls_iterator(urls);
    while(urls_iterator.hasNext()) {
        QUrl url = urls_iterator.next();
        if(url.isLocalFile()) {
            QString extension = url.fileName().split(".").last().toLower();
            if(extension == "m3u") {
                new_items += urlsToItems(m3uToUrls(url));
            } else {
                new_items += urlToItems(url);
            }
        }

        progress.setValue(progress.value() + 1);
        if(progress.wasCanceled()) break;
    }

    progress.setValue(urls.count());
    appendItems(new_items);
}

void PlayListModel::appendItems(QList<PlayListItem*> new_items) {
    if(new_items.isEmpty()) return;

    int start = items.size();
    beginInsertRows(QModelIndex(), start, start + new_items.size() - 1);
    items += new_items;
    endInsertRows();
}

QList<PlayListItem*> PlayListModel::urlToItems(QUrl url) {
    QList<PlayListItem*> new_items;

    if(url.isLocalFile()) {
        QFileInfo f(url.path());
        if(f.isDir()) {
            QDirIterator iterator(url.path(), QDirIterator::Subdirectories);
            while(iterator.hasNext()) {
                iterator.next();
                if(!iterator.fileInfo().isDir()) {
                    new_items += urlToItems(QUrl::fromLocalFile(iterator.filePath()));
                }
            }
        } else {
            PlayListItem* p = new PlayListItem(url);
            if(p->isValid()) {
                p->ensureHistogram();
                new_items += p;
            }
        }
    } else {
        new_items += new PlayListItem(url);
    }

    return new_items;
}

QList<PlayListItem*> PlayListModel::urlsToItems(QList<QUrl> urls) {
    QList<PlayListItem*> result;
    QListIterator<QUrl> urls_iterator(urls);
    while(urls_iterator.hasNext()) {
        result += urlToItems(urls_iterator.next());
    }
    return result;
}

QList<QUrl> PlayListModel::m3uToUrls(QUrl url) {
    QList<QUrl> urls;
    QFile m3u_file(url.path());
    if(m3u_file.open(QFile::ReadOnly)) {
        while(true) {
            QByteArray line = m3u_file.readLine();
            if(line.isEmpty()) break;

            line = line.trimmed();
            if(line.startsWith('#')) continue;
            if(line.isEmpty()) continue;

            urls.append(QUrl::fromUserInput(line));
        }
    }
    return urls;
}
