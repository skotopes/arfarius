#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QList>
#include <QUrl>
#include <QDebug>

#include "arfariusapplication.h"
#include "macmediakeys.h"
#include "playlistmodel.h"
#include "playlistitem.h"

#define CONFIG_VERSION 2

MainWindow::MainWindow(ArfariusApplication *application, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mac_media_keys(new MacMediaKeys(this)),
    player(new Player(this)),
    playlist(new PlayListModel(this)),
    current_item(nullptr)
{
    ui->setupUi(this);
    ui->playList->setModel(playlist);

    connect(application, SIGNAL( applicationStateChanged(Qt::ApplicationState) ), this, SLOT( applicationStateChanged(Qt::ApplicationState)));

    connect(mac_media_keys, SIGNAL( backward() ), playlist, SLOT(prevItem()));
    connect(mac_media_keys, SIGNAL( seekBackward() ), player, SLOT(seekBackward()));

    connect(mac_media_keys, SIGNAL( playPause() ), player, SLOT(playPause()));

    connect(mac_media_keys, SIGNAL( forward() ), playlist, SLOT(nextItem()));
    connect(mac_media_keys, SIGNAL( seekForward() ), player, SLOT(seekForward()));

    connect(ui->prevButton, SIGNAL(clicked()), playlist, SLOT(prevItem()));
    connect(ui->nextButton, SIGNAL(clicked()), playlist, SLOT(nextItem()));
    connect(ui->playButton, SIGNAL(clicked()), player, SLOT(playPause()));
    connect(ui->histogram, SIGNAL(clicked(float)), player, SLOT(seekToPercent(float)));

    connect(playlist, SIGNAL(itemUpdated(PlayListItem*)), this, SLOT(updateItem(PlayListItem*)));
    connect(playlist, SIGNAL(itemUpdated(PlayListItem*)), player, SLOT(updateItem(PlayListItem*)));

    connect(player, SIGNAL(trackEnded()), playlist, SLOT(nextItem()));

    connect(player, SIGNAL(stateUpdated(Player::State)), this, SLOT(updateState(Player::State)));
    connect(player, SIGNAL(progressUpdated(float)), ui->histogram, SLOT(updateProgress(float)));
    connect(player, SIGNAL(timeComboUpdated(QString)), ui->timeLabel, SLOT(setText(QString)));

    QSettings settings;
    if (settings.value("ConfigVersion", 0).toInt() < CONFIG_VERSION) {
        settings.clear();
        settings.setValue("ConfigVersion", CONFIG_VERSION);
    }

    restoreState(settings.value("MainWindow/state").toByteArray());
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    ui->playList->horizontalHeader()->restoreState(settings.value("PlayList/state").toByteArray());
    ui->playList->horizontalHeader()->restoreGeometry(settings.value("PlayList/geometry").toByteArray());

    connect(application, SIGNAL(fileDropped(QUrl)), playlist, SLOT(appendUrl(QUrl)));

    PlayListItem::ensurePath();
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

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left) {
        player->seekBackward(10.);
        event->accept();
    } else if (event->key() == Qt::Key_Right) {
        player->seekForward(10.);
        event->accept();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}


void MainWindow::updateState(Player::State s)
{
    qDebug() << this << "updateState()";
    switch (s) {
    case Player::PLAY:
        ui->playButton->setText("");
        break;
    case Player::PAUSE:
        ui->playButton->setText("");
        break;
    case Player::STOP:
        ui->playButton->setText("");
        break;
    default:
        break;
    }
}

void MainWindow::updateItem(PlayListItem* item)
{
    qDebug() << this << "updateItem()";
    if (current_item) {
        disconnect(current_item, SIGNAL(histogramUpdated()), this, SLOT(updateHistogram()));
    }
    current_item = item;

    if (current_item) {
        connect(current_item, SIGNAL(histogramUpdated()), this, SLOT(updateHistogram()));
    }

    updateHistogram();
}

void MainWindow::updateHistogram()
{
    qDebug() << this << "updateHistogram()";
    if (current_item) {
        auto r = ((QApplication*)QApplication::instance())->devicePixelRatio();
        QImage *image = current_item->getHistogrammImage(ui->histogram->width()*r, ui->histogram->height()*r);
        ui->histogram->updateImage(image);
    } else {
        ui->histogram->updateImage(nullptr);
    }
}

void MainWindow::applicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive) {
        show();
    }
}
