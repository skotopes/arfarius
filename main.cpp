#include "application.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Application a(argc, argv);
    a.setApplicationName("wmp");
    a.main_window->show();

    return a.exec();
}
