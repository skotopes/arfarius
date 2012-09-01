#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "playlist.h"

#include <QCloseEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug() << "Main created";
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    qDebug() << "Main destroyed";
    delete ui;
}

void MainWindow::setPlaylist(PlayList *p)
{
    ui->playList->setModel(p);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (!isMinimized()) {
        hide();
        e->ignore();
    } else {
        e->accept();
    }
}
