#include "arfariusapplication.h"
#include <QFileOpenEvent>

ArfariusApplication::ArfariusApplication(int & argc, char ** argv) :
    QApplication(argc, argv),
    decoder_thread_pool(this),
    analyze_thread_pool(this)
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

QThreadPool* ArfariusApplication::getDecoderThreadPool() {
    return &decoder_thread_pool;
}

QThreadPool* ArfariusApplication::getAnalyzeThreadPool() {
    return &analyze_thread_pool;
}
