#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QTableView>

class PlayListView : public QTableView
{
public:
    PlayListView(QWidget *parent);

protected:
    void contextMenuEvent(QContextMenuEvent *);
    void keyPressEvent(QKeyEvent *);
};

#endif // PLAYLISTVIEW_H
