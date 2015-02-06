#include "arfariusapplication.h"
#include "mainwindow.h"
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    ArfariusApplication a(argc, argv);
    a.setOrganizationName("Plooks Ltd");
    a.setOrganizationDomain("plooks.com");
    a.setApplicationName("Arfarius");
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);

    QFontDatabase::addApplicationFont("://fonts/FontAwesome.otf");

    MainWindow m(&a);
    m.show();

    return a.exec();
}
