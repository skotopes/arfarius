#include "player.h"

#include "playlist.h"
#include "playlistitem.h"
#include "avfile.h"

#include <QDebug>

Player::Player(QObject *parent) :
    QObject(parent), state(Player::STOP), playlist(0), current(0), current_connected(false),
    dac(), parameters(), sampleRate(0), bufferFrames(0)
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

    if (me->current_connected) {
        size_t ret = me->current->pull(buffer, nBufferFrames * 2);
        if (ret < nBufferFrames * 2 && !me->current->isDecoderRunning()) {
            me->next();
        }
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
    bufferFrames = 512;

    try {
        dac.openStream( &parameters, NULL, RTAUDIO_FLOAT32,
                        sampleRate, &bufferFrames,
                        &Player::callback, this );
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
        qDebug() << "Player::startStream()"<< "RtError:" << e.what();
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

void Player::updateState(Player::State s)
{
    state = s;
    emit stateChanged(state);
}

void Player::updateCurrent()
{
    PlayListItem *i = playlist->getCurrent();

    if (i) {
        current = new AVFile();
        current->open(i->getUrl().toLocal8Bit().constData());
        current->startDecoder();
        current_connected = true;
    }
}

void Player::disconnectCurrent()
{
    if (current) {
        if (current->isDecoderRunning())
            current->stopDecoder();
        current_connected = false;
        delete current; current = 0;
    }
}

void Player::playPause()
{
    if (state == Player::PLAY) {
        stopStream();
        updateState(Player::PAUSE);
    } else if (state == Player::PAUSE) {
        startStream();
        updateState(Player::PLAY);
    } else {
        if (playlist->next()) {
            updateCurrent();
            startStream();
            updateState(Player::PLAY);
        }
    }
}


void Player::stop()
{
    if (state == Player::STOP)
        return;

    disconnectCurrent();
    stopStream();
    updateState(Player::STOP);
}

void Player::next()
{
    if (playlist->next())
        updateCurrent();
    else
        stop();
}

void Player::prev()
{
    if (playlist->prev())
        updateCurrent();
    else
        stop();
}
