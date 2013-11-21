#include "wmpapplication.h"
#include <QFileOpenEvent>

WmpApplication::WmpApplication(int & argc, char ** argv) :
    QApplication(argc, argv)
{
}

bool WmpApplication::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::FileOpen:
        emit fileDropped(static_cast<QFileOpenEvent *>(event)->url());
        return true;
    default:
        return QApplication::event(event);
    }
}
