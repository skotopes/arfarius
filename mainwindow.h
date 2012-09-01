#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
