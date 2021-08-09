#include "player.h"

#include "arfariusapplication.h"
#include "playlistmodel.h"
#include "playlistitem.h"

#include "avexception.h"
#include "avfile.h"
#include "qcoreaudio.h"

#include <QtConcurrentRun>
#include <QSemaphore>
#include <QSettings>
#include <QTimer>
#include <QDebug>

QString formatTime(size_t time)
{
    int h,m,s;

    s = time % 60;
    m = time / 60 % 60;
    h = time / 60 / 60;

    if (h > 0)
        return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    else
        return QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));
}

Player::Player(QObject *parent) :
    QObject(parent),
    ca(new QCoreAudio(this)),
    file(nullptr), file_future(), eject_future(),
    ring(0), ring_semaphor(0), ring_size(0), samples_elapsed(0),
    state(Player::STOP), quiet(false)
{
    qRegisterMetaType<Player::State>("Player::State");

    ca->connectInput(this);
    _sample_rate = ca->getDeviceSampleRate();
    _channels = 2;

    ring_size = _sample_rate / 4;
    ring = new MemRing<av_sample_t>(ring_size);
    ring_semaphor = new QSemaphore(ring_size);
}

Player::~Player()
{
    if (file) {
        ejectFile();
    }

    if (ca) {
        stopStream();
    }

    if (ring) {
        delete ring; ring = nullptr;
    }

    if (ring_semaphor) {
        delete ring_semaphor; ring_semaphor = nullptr;
    }
}

const char * Player::getName()
{
    return "Player";
}

size_t Player::pull(float *buffer_ptr, size_t buffer_size)
{
    size_t ret = ring->pull(buffer_ptr, buffer_size);
    ring_semaphor->release(ret);

    return ret;
}

size_t Player::push(float *buffer_ptr, size_t buffer_size)
{
    size_t buffer_size_orig = buffer_size;
    while (buffer_size > 0) {
        // reduce buffer size if incoming buffer is bigger then 1/8 of the ring
        size_t granula_size = buffer_size;
        if (granula_size > ring_size/8) {
            granula_size = ring_size/8;
        }
        // advance semaphor and push data into the buffer
        ring_semaphor->acquire(granula_size);
        ring->push(buffer_ptr, granula_size);
        // adjust data pointer and size
        buffer_size -= granula_size;
        buffer_ptr  += granula_size;
    }

    samples_elapsed += buffer_size_orig;
    if (samples_elapsed > (_sample_rate * _channels / 8)) {
        samples_elapsed = 0;
        onProgressTimer();
    }

    return buffer_size_orig;
}

bool Player::startStream()
{
    ca->start(); 
    return true;
}

bool Player::stopStream()
{
    ca->stop();
    return true;
}

void Player::updateState(Player::State s)
{
    qDebug() << this << "updateState(): state changed to" << s;
    state = s;
    emit stateUpdated(state);
}

void Player::ejectFile()
{
    qDebug() << this << "ejectFile()";
    if (file) {
        file->cancelDecoding();

        eject_future = QtConcurrent::run(
            arfariusApp->getDecoderThreadPool(),
            [this](){
                av_sample_t *buffer = new av_sample_t[4096];
                while (state != Player::PLAY && file) {
                    qDebug() << this << "ejectFile(): pumping samples";
                    pull(buffer, 4096);
                }
                delete [] buffer ;
            }
        );
        file_future.waitForFinished();
        eject_future.waitForFinished();
    }
}

void Player::updateItem(PlayListItem *item)
{
    qDebug() << this << "updateItem()" << item;
    // skipping must suppress signals
    if (file) {
        quiet = true;
    }
    // set new item or stop
    if (item) {
        ejectFile();

        file = item->getAVFile();
        if (file == nullptr) {
            emit trackEnded();
            return;
        }

        file->setSamplerate(_sample_rate);
        file->setChannels(_channels);
        file->connectOutput(this);
        file_future = QtConcurrent::run(
            arfariusApp->getDecoderThreadPool(),
            [this](){
                file->decode();
                delete file; file = 0;
                if (quiet) {
                    quiet = false;
                } else {
                    emit trackEnded();
                }
            }
        );

        if (state != Player::PLAY) {
            startStream();
            updateState(Player::PLAY);
        }
    } else {
        stop();
    }
}

void Player::seekToPercent(float p)
{
    qDebug() << this << "seekTo()" << p;
    if (file) file->seekToPercent(p);
}

void Player::seekForward(float seconds)
{
    qDebug() << this << "seekForward" << seconds;
    if (file) file->seekForward(seconds);
}

void Player::seekBackward(float seconds)
{
    qDebug() << this << "seekBackward" << seconds;
    if (file) file->seekBackward(seconds);
}

void Player::playPause()
{
    qDebug() << this << "playPause()";
    if (state == Player::PLAY) {
        stopStream();
        updateState(Player::PAUSE);
    } else if (state == Player::PAUSE) {
        startStream();
        updateState(Player::PLAY);
    } else {
        startStream();
        updateState(Player::PLAY);
        if (!file) {
            emit trackEnded();
        }
    }
}

void Player::stop()
{
    qDebug() << this << "stop()";
    if (state == Player::STOP)
        return;

    ejectFile();
    stopStream();

    // wipe buffer content and restore semaphor
    ring->reset();
    int e = ring_semaphor->available();
    if (e < (int)ring_size) {
        ring_semaphor->release(ring_size - e);
    }
    // emit all neccesery signals
    updateState(Player::STOP);
    emit timeComboUpdated("=(-_-)=");
    emit progressUpdated(-1);
}

void Player::onProgressTimer()
{
    float file_duration = file->getDurationInSeconds();
    float file_samples = file->getDurationInSamples();
    float file_position = file->getPositionInPercents();
    float ring_offset = ring->readSpace()/_channels/file_samples;

    float percent = file_position - ring_offset;
    float position = percent * file_duration;

    emit progressUpdated(percent);
    emit timeComboUpdated(
        formatTime(position) + " [" +
        formatTime(file_duration) +"]"
    );
}
