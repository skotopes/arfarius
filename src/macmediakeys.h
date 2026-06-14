#ifndef MAC_MEDIA_KEYS_H
#define MAC_MEDIA_KEYS_H

#include <QObject>

class MacMediaKeys : public QObject {
    Q_OBJECT

public:
    explicit MacMediaKeys(QObject* parent = nullptr);
    ~MacMediaKeys();

    void setPlayingState(QString artist, QString title, bool playing);

signals:
    void play();
    void playPause();
    void pause();
    void stop();

    void forward();
    void backward();
};

#endif
