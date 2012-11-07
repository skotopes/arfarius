#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include "RtAudio.h"
#include "RtError.h"

class PlayList;
class AVFile;
class QMutex;

class Player : public QObject
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

    void setPlaylist(PlayList *p);

private:
    State state;
    PlayList *playlist;

    AVFile *track_current;
    QMutex *track_mutex;

    RtAudio dac;
    RtAudio::StreamParameters parameters;
    unsigned int sampleRate;
    unsigned int bufferFrames;
    double streamTime;

    void openStream();
    void startStream();
    void stopStream();
    void closeStream();

    void updateState(Player::State);
    void updateCurrent();
    void disconnectCurrent();

    void emitNewPlayPointer(float p);

    static int callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData );

signals:
    void stateChanged(Player::State);
    void newPlayPointer(float p);

public slots:
    void setPlayPointer(float p);
    void playPause();
    void stop();
    void next();
    void prev();
};

#endif // PLAYER_H
