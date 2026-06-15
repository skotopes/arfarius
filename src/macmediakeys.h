#ifndef MAC_MEDIA_KEYS_H
#define MAC_MEDIA_KEYS_H

#include <QObject>

class MacMediaKeys : public QObject {
    Q_OBJECT

public:
    explicit MacMediaKeys(QObject* parent = nullptr);
    ~MacMediaKeys();

    MacMediaKeys(const MacMediaKeys&) = delete;
    MacMediaKeys& operator=(const MacMediaKeys&) = delete;
    MacMediaKeys(MacMediaKeys&&) = delete;
    MacMediaKeys& operator=(MacMediaKeys&&) = delete;

    void setPlayingState(const QString& artist, const QString& title, bool playing);

signals:
    void play();
    void playPause();
    void pause();
    void stop();

    void forward();
    void backward();
};

#endif
