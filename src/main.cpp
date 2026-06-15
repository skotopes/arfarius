#include "arfariusapplication.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    ArfariusApplication a(argc, argv);
    a.setOrganizationName("Plooks Ltd");
    a.setOrganizationDomain("plooks.com");
    a.setApplicationName("Arfarius");

    MainWindow m(&a);
    m.show();

    return a.exec();
}
