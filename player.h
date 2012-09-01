#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = 0);
    ~Player();

signals:

public slots:
    void playPause();
    void stop();
    void next();
    void prev();
};

#endif // PLAYER_H
