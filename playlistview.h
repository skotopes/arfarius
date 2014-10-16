#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QTableView>

class PlayListView : public QTableView
{
public:
    PlayListView(QWidget *parent);

protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

    void contextMenuEvent(QContextMenuEvent *);
    void keyPressEvent(QKeyEvent *);

    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

};

#endif // PLAYLISTVIEW_H
