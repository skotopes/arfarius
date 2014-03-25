#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

#include "avobject.h"
#include "memring.h"

class PlayListModel;
class AVFile;
class QSemaphore;
class QCoreAudio;

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

    void setPlaylist(PlayListModel *p);

    virtual const char * getName();
    virtual size_t pull(float *buffer_ptr, size_t buffer_size);
    virtual size_t push(float *buffer_ptr, size_t buffer_size);

private:
    QCoreAudio *ca;
    PlayListModel *playlist;
    AVFile *file;
    QFuture<void> file_future;
    QFutureWatcher<void> file_future_watcher;
    QFuture<void> histogram_future;
    QFutureWatcher<void> histogram_future_watcher;
    MemRing<av_sample_t> *ring;
    QSemaphore *ring_semaphor;
    size_t ring_size;
    State state;
    size_t cnt;

    // Stream and DAC
    bool startStream();
    bool stopStream();

    // Player state
    void updateState(Player::State);
    void loadFile();
    void ejectFile();

signals:
    void stateUpdated(Player::State);
    void timeComboUpdated(QString);
    void progressUpdated(float);
    void histogramUpdated(QImage *);

public slots:
    void seekTo(float p);
    void playPause();
    void stop();
    void next();
    void prev();
    QImage *analyze();

private slots:
    void onTrackEnd();
};

#endif // PLAYER_H
