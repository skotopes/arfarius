#include "playlistview.h"

#include <QMenu>
#include <QModelIndex>
#include <QContextMenuEvent>

#include <QDebug>

PlayListView::PlayListView(QWidget *parent) : QTableView(parent)
{
}

void PlayListView::contextMenuEvent(QContextMenuEvent * event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        QMenu *menu = new QMenu(this);
        menu->addAction(QString("Row %1 - Col %2 was clicked on").arg(index.row()).arg(index.column()));
        menu->exec(QCursor::pos());
        QTableView::contextMenuEvent(event);
    } else {
        event->ignore();
    }
}

void PlayListView::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        QModelIndexList indexes = selectionModel()->selection().indexes();
        QSet<int> itr_set;
        for (int i = 0; i < indexes.count(); ++i) {
            QModelIndex index = indexes.at(i);
            itr_set.insert(index.row());
        }

        QList<int> itr_list = itr_set.toList();
        qSort(itr_list);
        QListIterator<int> itr_list_i(itr_list);
        itr_list_i.toBack();
        while (itr_list_i.hasPrevious()) {
            model()->removeRow(itr_list_i.previous());
        }
    }

    QTableView::keyPressEvent(event);
}
