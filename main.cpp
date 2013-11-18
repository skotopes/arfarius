#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_MACX
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 ) {
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif

    QApplication::setOrganizationName("White-label Ltd");
    QApplication::setOrganizationDomain("white-label.ru");
    QApplication::setApplicationName("wmp");

    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    MainWindow m;
    m.show();
    return a.exec();
}
