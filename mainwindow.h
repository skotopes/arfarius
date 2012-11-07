#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QUrl>

#include "player.h"

namespace Ui {
    class MainWindow;
}

class PlayList;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setPlaylist(PlayList *p);

protected:
    void closeEvent(QCloseEvent *e);
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
    Ui::MainWindow *ui;

signals:
    void droppedUrls(QList<QUrl> urls);
    void playPause();
    void next();
    void prev();

    void newPlayPointer(float p);

public slots:
    void updateState(Player::State);
    void updatePlayPointer(float p);
};

#endif // MAINWINDOW_H
