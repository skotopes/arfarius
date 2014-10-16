#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "player.h"

namespace Ui {
    class MainWindow;
}

class ArfariusApplication;
class PlayListModel;
class PlayListItem;
class MacSupport;
class Collection;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(ArfariusApplication *application, QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e);

private:
    Ui::MainWindow *ui;
    MacSupport *platform_support;
    Player *player;
    PlayListModel *playlist;
    PlayListItem *current_item;

private slots:
    void updateState(Player::State);
    void updateItem(PlayListItem*);
    void updateHistogram();

};

#endif // MAINWINDOW_H
