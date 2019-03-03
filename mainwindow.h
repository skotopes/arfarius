#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "player.h"

namespace Ui {
    class MainWindow;
}

class QtAwesome;
class ArfariusApplication;
class PlayListModel;
class PlayListItem;
class MacMediaKeys;
class Collection;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(ArfariusApplication *application, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e);
    void keyPressEvent(QKeyEvent *);

private:
    QtAwesome *awesome;
    Ui::MainWindow *ui;
    MacMediaKeys *mac_media_keys;
    Player *player;
    PlayListModel *playlist;
    PlayListItem *current_item;

private slots:
    void updateState(Player::State);
    void updateItem(PlayListItem*);
    void updateHistogram();
    void applicationStateChanged(Qt::ApplicationState);

};

#endif // MAINWINDOW_H
