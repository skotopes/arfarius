#include "wmpapplication.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_MACX
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 ) {
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif

    WmpApplication a(argc, argv);
    a.setOrganizationName("Plooks Ltd");
    a.setOrganizationDomain("plooks.com");
    a.setApplicationName("wmp");
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);

    MainWindow m(&a);
    m.show();

    return a.exec();
}
