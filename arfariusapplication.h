#ifndef WMPAPPLICATION_H
#define WMPAPPLICATION_H

#include <QApplication>
#include <QUrl>

class ArfariusApplication : public QApplication
{
    Q_OBJECT
public:
    explicit ArfariusApplication(int &argc, char **argv);
    bool event(QEvent *);

signals:
    void fileDropped(QUrl);

public slots:

};

#endif // WMPAPPLICATION_H
