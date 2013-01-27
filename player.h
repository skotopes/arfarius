#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include "RtAudio.h"
#include "RtError.h"

#include "avfile.h"

class PlayListModel;
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

    void setPlaylist(PlayListModel *p);

private:
    State state;
    PlayListModel *playlist;

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

    void emitNewPlayProgress(AVFile::Progress p);

    static int callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData );

signals:
    void stateChanged(Player::State);
    void newPlayProgress(AVFile::Progress);

public slots:
    void setPlayPointer(float p);
    void playPause();
    void stop();
    void next();
    void prev();
};

#endif // PLAYER_H
