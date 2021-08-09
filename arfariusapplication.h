#ifndef WMPAPPLICATION_H
#define WMPAPPLICATION_H

#include <QApplication>
#include <QThreadPool>
#include <QUrl>

#define arfariusApp (static_cast<ArfariusApplication *>(QCoreApplication::instance()))

class ArfariusApplication : public QApplication
{
    Q_OBJECT

    QThreadPool decoder_thread_pool;
    QThreadPool analyze_thread_pool;

public:
    explicit ArfariusApplication(int &argc, char **argv);
    bool event(QEvent *);

    QThreadPool *getDecoderThreadPool();
    QThreadPool *getAnalyzeThreadPool();

signals:
    void fileDropped(QUrl);

public slots:

};

#endif // WMPAPPLICATION_H
