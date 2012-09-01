#include "application.h"

#include "macsupport.h"
#include "mainwindow.h"
#include "player.h"
#include "playlist.h"

#include <QColor>

Application::Application(int argc, char *argv[])
    : QApplication(argc, argv), platform_support(0), main_window(0), player(0), playlist(0)
{
    platform_support = new MacSupport();
    main_window = new MainWindow();
    player = new Player();
    playlist = new PlayList();

    main_window->setPlaylist(playlist);

    platform_support->setDockBadge("trolo");

    platform_support->setCustomBorderColor(QColor(83,83,83));
    platform_support->setCustomTitleColor(QColor(226,226,226));
    platform_support->installCustomFrame();

    platform_support->setDockOverlay(0);

    connect(platform_support, SIGNAL( dockClicked() ), main_window, SLOT( show() ));
}

Application::~Application()
{
}
