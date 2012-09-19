#include "player.h"
#include "playlist.h"
#include "avfile.h"

#include <QDebug>

Player::Player(QObject *parent) :
    QObject(parent), state(Player::STOP), playlist(0), current(0), dac(), parameters(), sampleRate(0), bufferFrames(0)
{
    openStream();
}

Player::~Player()
{
    stopStream();
    closeStream();

    if (current)
        delete current;
}

int Player::callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status, void *userData )
{
    Q_UNUSED(status)
    Q_UNUSED(streamTime)
    Q_UNUSED(inputBuffer)

    Player *me = reinterpret_cast<Player *>(userData);
    float *buffer = reinterpret_cast<float *>(outputBuffer);

    if (me->current) {
        me->current->pull(buffer, nBufferFrames * 2);
    }

    return 0;
}

void Player::setPlaylist(PlayList *p)
{
    playlist = p;
}

void Player::openStream()
{
    if ( dac.getDeviceCount() < 1 ) {
        return; // todo show error dialog
    }

    // TODO: load it from setting please
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = 2;
    parameters.firstChannel = 0;

    sampleRate = 44100;
    bufferFrames = 64;

    try {
        dac.openStream( &parameters, NULL, RTAUDIO_FLOAT32,
                        sampleRate, &bufferFrames,
                        &Player::callback, reinterpret_cast<void *>(this) );
    } catch ( RtError& e ) {
        qDebug() << "Player: open stream error, " << e.what();
        return;
    }

}

void Player::startStream()
{
    try {
        dac.startStream();
    } catch ( RtError& e ) {
        qDebug() << "Player: start stream error, " << e.what();
        return;
    }

}

void Player::stopStream()
{
    try {
        // Stop the stream
        dac.stopStream();
    }
    catch (RtError& e) {
        qDebug() << e.what();
    }
}

void Player::closeStream()
{
    if (dac.isStreamOpen())
        dac.closeStream();
}

void Player::playPause()
{
    if (state == Player::PLAY) {
        stopStream();
        state = Player::PAUSE;
    } else if (state == Player::PAUSE) {
        startStream();
        state = Player::PLAY;
    } else {
        if (playlist->hasNext()) {
            QUrl u = playlist->getNext();
            current = new AVFile();
            current->open(u.toString().toLocal8Bit().constData());
            current->startDecoder();
            startStream();
            state = Player::PLAY;
        }
    }
}

void Player::stop()
{
    if (state == Player::STOP)
        return;

    stopStream();
    delete current;
    current = 0;
}

void Player::next()
{

}

void Player::prev()
{

}
