#ifndef MAC_MEDIA_KEYS_H
#define MAC_MEDIA_KEYS_H

#include <QObject>

class QColor;

class MacMediaKeys : public QObject
{
    Q_OBJECT
public:
    MacMediaKeys(QObject *parent = 0);
    virtual ~MacMediaKeys();

    void onKeyEvent(int keycode, int keystate);

signals:
    void prev();
    void play();
    void next();
};

#endif
