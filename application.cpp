#include "application.h"

#include "macsupport.h"
#include "mainwindow.h"
#include "player.h"
#include "playlist.h"

#include <QColor>

Application::Application(int argc, char *argv[])
    : QApplication(argc, argv), platform_support(0), main_window(0), player(0), playlist(0)
{
    // Allocate
    platform_support = new MacSupport();
    main_window = new MainWindow();
    playlist = new PlayList();
    player = new Player();

    // Connect
    main_window->setPlaylist(playlist);
    player->setPlaylist(playlist);

    platform_support->setCustomBorderColor(QColor(83,83,83));
    platform_support->setCustomTitleColor(QColor(226,226,226));
    platform_support->installCustomFrame();

    // Connect signals
    connect(platform_support, SIGNAL( dockClicked() ), main_window, SLOT( show() ));
    connect(platform_support, SIGNAL( playPause() ), player, SLOT( playPause() ));

    connect(main_window, SIGNAL( droppedUrls(QList<QUrl>) ), playlist, SLOT( appendUrls(QList<QUrl>) ));
    connect(main_window, SIGNAL( playPause() ), player, SLOT( playPause() ));
    connect(main_window, SIGNAL( next() ), player, SLOT( next() ));
    connect(main_window, SIGNAL( prev() ), player, SLOT( prev() ));

    connect(player, SIGNAL( stateChanged(Player::State) ), main_window, SLOT( updateState(Player::State) ));
}

Application::~Application()
{
}
