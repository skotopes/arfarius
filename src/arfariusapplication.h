#ifndef ARFARIUSAPPLICATION_H
#define ARFARIUSAPPLICATION_H

#include <QApplication>
#include <QThreadPool>
#include <QUrl>

#define arfariusApp (static_cast<ArfariusApplication*>(QCoreApplication::instance()))

class ArfariusApplication : public QApplication {
    Q_OBJECT

    QThreadPool decoder_thread_pool;
    QThreadPool analyze_thread_pool;

public:
    explicit ArfariusApplication(int& argc, char** argv);

    QThreadPool* getDecoderThreadPool();
    QThreadPool* getAnalyzeThreadPool();

signals:
    void fileDropped(QUrl);

protected:
    bool event(QEvent*) override;
};

#endif // ARFARIUSAPPLICATION_H
