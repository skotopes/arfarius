#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

#include "avobject.h"
#include "memring.h"

class PlayListItem;
class AVFile;
class QSemaphore;
class QCoreAudio;
class QTimer;

class Player : public QObject, public AVObject
{
    Q_OBJECT

public:
    enum State {
        STOP,
        PLAY,
        PAUSE
    };

    explicit Player(QObject *parent = 0);
    virtual ~Player();

    virtual const char * getName();
    virtual size_t pull(float *buffer_ptr, size_t buffer_size);
    virtual size_t push(float *buffer_ptr, size_t buffer_size);

private:
    QCoreAudio *ca;
    AVFile *file;
    QFuture<void> file_future;
    QFuture<void> eject_future;
    MemRing<av_sample_t> *ring;
    QSemaphore *ring_semaphor;
    size_t ring_size;
    QTimer *progress_timer;
    State state;
    bool quiet;

    // Stream and DAC
    bool startStream();
    bool stopStream();

    // Player state
    void updateState(Player::State);
    void ejectFile();

signals:
    void trackEnded();
    void stateUpdated(Player::State);
    void timeComboUpdated(QString);
    void progressUpdated(float);

public slots:
    void updateItem(PlayListItem *);
    void seekTo(float p);
    void playPause();
    void stop();

private slots:
    void onProgressTimer();
};

#endif // PLAYER_H
