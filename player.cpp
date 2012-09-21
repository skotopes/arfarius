#include "player.h"

#include "playlist.h"
#include "playlistitem.h"
#include "avfile.h"

#include <QDebug>

Player::Player(QObject *parent) :
    QObject(parent), state(Player::STOP), playlist(0),
    track_current(0), track_next(0), track_change(false),
    dac(), parameters(), sampleRate(0), bufferFrames(0)
{
    openStream();
}

Player::~Player()
{
    stopStream();
    closeStream();


    delete track_current;
    delete track_next;
}

int Player::callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status, void *userData )
{
    Q_UNUSED(status)
    Q_UNUSED(streamTime)
    Q_UNUSED(inputBuffer)

    Player *me = reinterpret_cast<Player *>(userData);
    float *buffer = reinterpret_cast<float *>(outputBuffer);

    if (me->track_current) {
        size_t ret = me->track_current->pull(buffer, nBufferFrames * 2);

        if (ret < nBufferFrames * 2 && !me->track_current->isDecoderRunning()) {
            me->next();
        }
    }

    if (me->track_change) {
        if (me->track_current && me->track_current->isDecoderRunning())
            me->track_current->stopDecoder();

        delete me->track_current;
        me->track_current = me->track_next;
        me->track_next = 0;
        me->track_change = false;
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
        qDebug() << "Player::openStream()" << "No output found";
        return;
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
        qDebug() << "Player::openStream()" << "RtError" << e.what();
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
        dac.stopStream();
    } catch (RtError& e) {
        qDebug() << "Player::stopStream()" << "RtError:" << e.what();
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
        track_next = new AVFile();
        track_next->open(i->getUrl().toLocal8Bit().constData());
        track_next->startDecoder();
        track_change = true;
    }
}

void Player::disconnectCurrent()
{
    track_next = 0;
    track_change = true;
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
