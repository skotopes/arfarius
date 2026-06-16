#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QTableView>

class PlayListView : public QTableView {
    Q_OBJECT
public:
    explicit PlayListView(QWidget* parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
};

#endif // PLAYLISTVIEW_H
