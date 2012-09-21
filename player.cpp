#include "player.h"

#include "playlist.h"
#include "playlistitem.h"
#include "avfile.h"
#include "avmutex.h"

#include <QDebug>

Player::Player(QObject *parent) :
    QObject(parent), state(Player::STOP), playlist(0), current(0), current_mutex(0),
    dac(), parameters(), sampleRate(0), bufferFrames(0)
{
    current_mutex = new AVMutex();
    openStream();
}

Player::~Player()
{
    stopStream();
    closeStream();

    delete current;
    delete current_mutex;
}

int Player::callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status, void *userData )
{
    Q_UNUSED(status)
    Q_UNUSED(streamTime)
    Q_UNUSED(inputBuffer)

    Player *me = reinterpret_cast<Player *>(userData);
    float *buffer = reinterpret_cast<float *>(outputBuffer);

    me->current_mutex->lock();
    if (me->current) {
        size_t ret = me->current->pull(buffer, nBufferFrames * 2);
        if (ret < nBufferFrames * 2 && !me->current->isDecoderRunning()) {
            me->next();
        }
    }
    me->current_mutex->unlock();

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
    disconnectCurrent();

    PlayListItem *i = playlist->getCurrent();

    if (i) {
        current = new AVFile();
        current->open(i->getUrl().toLocal8Bit().constData());
        current->startDecoder();
    }
}

void Player::disconnectCurrent()
{
    if (current) {
        if (current->isDecoderRunning())
            current->stopDecoder();

        current_mutex->lock();
        delete current; current = 0;
        current_mutex->unlock();
    }
}

void Player::playPause()
{
    qDebug() << "Player::playPause()";
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
    qDebug() << "Player::stop()";
    if (state == Player::STOP)
        return;

    disconnectCurrent();
    stopStream();
    updateState(Player::STOP);
}

void Player::next()
{
    qDebug() << "Player::next()";
    if (playlist->next())
        updateCurrent();
    else
        stop();
}

void Player::prev()
{
    qDebug() << "Player::prev()";
    if (playlist->prev())
        updateCurrent();
    else
        stop();
}
