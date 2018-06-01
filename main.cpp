#include "arfariusapplication.h"
#include "mainwindow.h"
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    ArfariusApplication a(argc, argv);
    a.setOrganizationName("Plooks Ltd");
    a.setOrganizationDomain("plooks.com");
    a.setApplicationName("Arfarius");

    MainWindow m(&a);
    m.show();

    return a.exec();
}
