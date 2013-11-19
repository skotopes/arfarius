#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QMimeData>
#include <QSettings>
#include <QList>
#include <QUrl>
#include <QDebug>

#include "macsupport.h"
#include "playlistmodel.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    platform_support(new MacSupport(this)),
    playlist(new PlayListModel(this)),
    player(new Player(this))
{
    ui->setupUi(this);
    ui->playList->setModel(playlist);
    player->setPlaylist(playlist);

    connect(platform_support, SIGNAL( dockClicked() ), this, SLOT( show() ));

    connect(ui->prevButton, SIGNAL(clicked()), player, SLOT(prev()));
    connect(ui->playButton, SIGNAL(clicked()), player, SLOT(playPause()));
    connect(ui->nextButton, SIGNAL(clicked()), player, SLOT(next()));
    connect(ui->histogram, SIGNAL(newPlayPointer(float)), player, SLOT(seekTo(float)));

    connect(player, SIGNAL(progressUpdated(float)), ui->histogram, SLOT(updatePlayProgress(float)));
    connect(player, SIGNAL(histogramUpdated(QImage*)), ui->histogram, SLOT(updateImage(QImage*)));
    connect(player, SIGNAL(stateUpdated(Player::State)), this, SLOT(updateState(Player::State)));
    connect(player, SIGNAL(timeComboUpdated(QString)), ui->timeLabel, SLOT(setText(QString)));

    QSettings settings;
    restoreState(settings.value("MainWindow/state").toByteArray());
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    ui->playList->horizontalHeader()->restoreState(settings.value("PlayList/state").toByteArray());
    ui->playList->horizontalHeader()->restoreGeometry(settings.value("PlayList/geometry").toByteArray());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    QSettings settings;
    settings.setValue("PlayList/state", ui->playList->horizontalHeader()->saveState());
    settings.setValue("PlayList/geometry", ui->playList->horizontalHeader()->saveGeometry());
    settings.setValue("MainWindow/state", saveState());
    settings.setValue("MainWindow/geometry", saveGeometry());

    if (!isMinimized()) {
        hide();
        e->ignore();
    } else {
        e->accept();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    e->acceptProposedAction();
    playlist->appendUrls(urls);
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
