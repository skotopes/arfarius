#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

#include "avobject.h"
#include "memring.h"
#include "RtAudio.h"

class PlayListModel;
class AVFile;
class QSemaphore;

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
    RtAudio *dac;
    PlayListModel *playlist;
    AVFile *file;
    QFuture<void> file_future;
    QFutureWatcher<void> file_future_watcher;
    MemRing<av_sample_t> *buffer;
    QSemaphore *buffer_semaphor;
    State state;
    size_t cnt;
    volatile bool stopping;

    static int callback(void *outputBuffer, void *inputBuffer,
                        unsigned int nBufferFrames, double streamTime,
                        RtAudioStreamStatus status, void *userData);

    // Stream and DAC
    void openDAC();
    void startStream();
    void stopStream();
    void closeDAC();

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
