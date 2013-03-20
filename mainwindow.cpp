#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "playlistmodel.h"

#include <QCloseEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug() << "Main created";
    ui->setupUi(this);

    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    restoreState(settings.value("MainWindow/state").toByteArray());

    connect(ui->playButton, SIGNAL( clicked() ), this, SIGNAL( playPause() ));
    connect(ui->prevButton, SIGNAL( clicked() ), this, SIGNAL( prev() ));
    connect(ui->nextButton, SIGNAL( clicked() ), this, SIGNAL( next() ));

    connect(ui->histogram, SIGNAL(newPlayPointer(float)), this, SIGNAL(newPlayPointer(float)));
}

MainWindow::~MainWindow()
{
    delete ui;
    qDebug() << "Main destroyed";
}

void MainWindow::setPlaylist(PlayListModel *p)
{
    ui->playList->setModel(p);
    ui->playList->horizontalHeader()->restoreGeometry(settings.value("PlayList/geometry").toByteArray());
    ui->playList->horizontalHeader()->restoreState(settings.value("PlayList/state").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/state", saveState());

    settings.setValue("PlayList/geometry", ui->playList->horizontalHeader()->saveGeometry());
    settings.setValue("PlayList/state", ui->playList->horizontalHeader()->saveState());

    settings.sync(); // force save

    if (!isMinimized()) {
        hide();
        e->ignore();
    } else {
        e->accept();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    qDebug() << e->mimeData()->formats();
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    qDebug() << urls;
    e->acceptProposedAction();
    emit droppedUrls(urls);
}

void MainWindow::updateState(Player::State s)
{
    switch (s) {
    case Player::PLAY:
        ui->playButton->setIcon(QIcon(":/images/pause.svg"));
        break;
    case Player::PAUSE:
        ui->playButton->setIcon(QIcon(":/images/play.svg"));
        break;
    case Player::STOP:
        ui->playButton->setIcon(QIcon(":/images/play.svg"));
        break;
    default:
        break;
    }
}

void MainWindow::updatePlayProgress(AVFile::Progress p)
{
    ui->histogram->updatePlayProgress(p);
    ui->timeWidget->updatePlayProgress(p);
}
