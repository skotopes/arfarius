#include "player.h"

#include "playlist.h"
#include "playlistitem.h"

#include <QMutex>
#include <QDebug>

Player::Player(QObject *parent) :
    QObject(parent), state(Player::STOP), playlist(0),
    track_current(0), track_mutex(new QMutex(QMutex::Recursive)),
    dac(), parameters(), sampleRate(0), bufferFrames(0)
{
    openStream();
    qRegisterMetaType<Player::State>("Player::State");
}

Player::~Player()
{
    stopStream();
    closeStream();

    delete track_current;
}

int Player::callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status, void *userData )
{
    Q_UNUSED(status)
    Q_UNUSED(streamTime)
    Q_UNUSED(inputBuffer)

    Player *me = reinterpret_cast<Player *>(userData);
    float *buffer = reinterpret_cast<float *>(outputBuffer);

    me->track_mutex->lock();
    if (me->track_current) {
        size_t ret = me->track_current->pull(buffer, nBufferFrames * 2);
        if (ret < nBufferFrames * 2 && me->track_current->isEOF()) {
            if (me->playlist->next()) {
                me->updateCurrent();
            } else {
                me->updateState(Player::STOP);
                me->track_mutex->unlock();
                return 1;
            }
        }

        if (streamTime > (me->streamTime + .25)) {
            me->streamTime = streamTime;
            me->emitNewPlayProgress(me->track_current->getProgress());
        }
    }
    me->track_mutex->unlock();

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
    qDebug() << "Player::updateState()" << "state changed to:" << s;
    emit stateChanged(state);
}

void Player::updateCurrent()
{
    PlayListItem *i = playlist->getCurrent();

    if (i) {
        AVFile *tn, *to;

        tn = new AVFile();
        tn->open(i->getUrl().toLocal8Bit().constData());
        tn->startDecoder();

        track_mutex->lock();
        to = track_current;
        track_current = tn;
        track_mutex->unlock();

        if (to && to->isDecoderRunning())
            to->stopDecoder();
        delete to;

        if (state == Player::PAUSE) {
            startStream();
            updateState(Player::PLAY);
        }
    }
}

void Player::disconnectCurrent()
{
    AVFile *to;

    track_mutex->lock();
    to = track_current;
    track_current = 0;
    track_mutex->unlock();

    if (to) {

        delete to;
    }
}

void Player::emitNewPlayProgress(AVFile::Progress p)
{
    emit newPlayProgress(p);
}

void Player::setPlayPointer(float p)
{
    track_mutex->lock();
    if (!track_current->isEOF()) {
        track_current->seekToPositionPercent(p);
    }
    track_mutex->unlock();
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
    qDebug() << "Player::stop() end";
    updateState(Player::STOP);
}

void Player::next()
{
    qDebug() << "Player::next()";
    if (state == Player::STOP)
        return;

    if (playlist->next())
        updateCurrent();
    else
        stop();
}

void Player::prev()
{
    qDebug() << "Player::prev()";
    if (state == Player::STOP)
        return;

    if (playlist->prev())
        updateCurrent();
    else
        stop();
}
