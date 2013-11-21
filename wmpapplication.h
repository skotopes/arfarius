#ifndef WMPAPPLICATION_H
#define WMPAPPLICATION_H

#include <QApplication>
#include <QUrl>

class WmpApplication : public QApplication
{
    Q_OBJECT
public:
    explicit WmpApplication(int &argc, char **argv);
    bool event(QEvent *);

signals:
    void fileDropped(QUrl);

public slots:

};

#endif // WMPAPPLICATION_H
