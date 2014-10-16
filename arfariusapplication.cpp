#include "arfariusapplication.h"
#include <QFileOpenEvent>

ArfariusApplication::ArfariusApplication(int & argc, char ** argv) :
    QApplication(argc, argv)
{
}

bool ArfariusApplication::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::FileOpen:
        emit fileDropped(static_cast<QFileOpenEvent *>(event)->url());
        return true;
    default:
        return QApplication::event(event);
    }
}
