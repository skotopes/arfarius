#include "playlistview.h"

#include <QDrag>
#include <QMenu>
#include <QMimeData>
#include <QModelIndex>
#include <QMouseEvent>
#include <QProgressDialog>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QFileInfo>
#include <QSet>

#include <QDebug>
#include "playlistmodel.h"
#include "playlistitem.h"

PlayListView::PlayListView(QWidget* parent)
    : QTableView(parent) {
    setDragDropMode(QTableView::DragDrop);
    setDropIndicatorShown(true);
    setDragEnabled(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setAcceptDrops(true);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropOverwriteMode(false);
}

void PlayListView::contextMenuEvent(QContextMenuEvent* event) {
    qDebug() << this << "contextMenuEvent()" << event;
    QModelIndex index = indexAt(event->pos());
    if(index.isValid()) {
        QMenu menu(this);
        auto* showAction = menu.addAction("Show in Finder");
        QAction* activated = menu.exec(QCursor::pos());
        if(activated == showAction) {
            PlayListModel* playlist = dynamic_cast<PlayListModel*>(model());
            if(playlist && index.row() >= 0 && index.row() < playlist->rowCount()) {
                PlayListItem* item = playlist->items.value(index.row());
                if(item) {
                    QString filePath = item->getUrlStringLocal();
                    if(!filePath.isEmpty()) {
                        QFileInfo fi(filePath);
                        QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
                    }
                }
            }
        }
    } else {
        QTableView::contextMenuEvent(event);
    }
}

void PlayListView::keyPressEvent(QKeyEvent* event) {
    qDebug() << this << "keyPressEvent()" << event;

    if(state() == QTableView::EditingState) {
        QTableView::keyPressEvent(event);
        return;
    }

    if(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        QModelIndexList indexes = selectionModel()->selection().indexes();
        QSet<int> itr_set;
        for(int i = 0; i < indexes.count(); ++i) {
            QModelIndex index = indexes.at(i);
            itr_set.insert(index.row());
        }

        QList<int> itr_list = itr_set.values();
        std::sort(itr_list.begin(), itr_list.end());
        QListIterator<int> itr_list_i(itr_list);
        itr_list_i.toBack();
        while(itr_list_i.hasPrevious()) {
            model()->removeRow(itr_list_i.previous());
        }
        event->accept();
    } else if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QModelIndexList selection = selectionModel()->selection().indexes();
        if(!selection.isEmpty()) {
            QModelIndex index = selection.first();
            if(index.isValid()) {
                PlayListModel* playlist = dynamic_cast<PlayListModel*>(model());
                if(playlist) {
                    playlist->clickedItem(index);
                } else {
                    qWarning() << this << "Incompatible model for drag and drop event";
                }
            }
        }
        event->accept();
    } else if(
        event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_L) {
        event->ignore();
    } else {
        QTableView::keyPressEvent(event);
    }
}

void PlayListView::mousePressEvent(QMouseEvent* event) {
    qDebug() << this << "mousePressEvent()" << event;

    if(event->modifiers() & Qt::AltModifier) {
        QModelIndex index = indexAt(event->pos());
        if(index.isValid()) {
            edit(index);
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        QTableView::mousePressEvent(event);
    }
}

void PlayListView::mouseDoubleClickEvent(QMouseEvent* event) {
    qDebug() << this << "mouseDoubleClickEvent()" << event;

    QModelIndex index = indexAt(event->pos());
    if(index.isValid()) {
        PlayListModel* playlist = dynamic_cast<PlayListModel*>(model());
        if(playlist) {
            playlist->clickedItem(index);
        } else {
            qWarning() << this << "Incompatible model for drag and drop event";
        }
        event->accept();
    } else {
        event->ignore();
    }
}
