#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "player.h"

namespace Ui {
    class MainWindow;
}

class PlayListModel;
class MacSupport;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e);
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
    Ui::MainWindow *ui;
    MacSupport *platform_support;
    PlayListModel *playlist;
    Player *player;

public slots:
    void updateState(Player::State);
};

#endif // MAINWINDOW_H
