#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>

#include "RtAudio.h"
#include "RtError.h"

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QObject *parent = 0);
    ~Player();

    void openStream();
    void startStream();
    void stopStream();
    void closeStream();

private:
    RtAudio dac;
    RtAudio::StreamParameters parameters;
    unsigned int sampleRate;
    unsigned int bufferFrames;

    static int callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData );

signals:

public slots:
    void playPause();
    void stop();
    void next();
    void prev();
};

#endif // PLAYER_H
