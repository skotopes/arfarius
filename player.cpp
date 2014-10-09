#include "player.h"

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
    ca(new QCoreAudio(this)), playlist(nullptr),
    file(nullptr), file_future(), file_future_watcher(),
    ring(0), ring_semaphor(0), ring_size(0),
    progress_timer(nullptr), state(Player::STOP)
{
    qRegisterMetaType<Player::State>("Player::State");
    connect(&file_future_watcher, SIGNAL(finished()), this, SLOT(onTrackEnd()));

    ca->connectInput(this);
    _sample_rate = ca->getDeviceSampleRate();
    _channels = 2;

    ring_size = ca->getDeviceBufferSize();
    ring_size *= 8;
    ring = new MemRing<av_sample_t>(ring_size);
    ring_semaphor = new QSemaphore(ring_size);

    progress_timer = new QTimer(this);
    progress_timer->setInterval(250);
    connect(progress_timer, SIGNAL(timeout()), this, SLOT(onProgressTimer()));
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
        delete ring;
        ring = 0;
    }
}

void Player::setPlaylist(PlayListModel *p)
{
    playlist = p;
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
    // check if we have enough space in the ring
    // reduce buffer size if incoming buffer is bigger then 1/8
    if (buffer_size > ring_size/8) {
        buffer_size = ring_size/8;
    }

    ring_semaphor->acquire(buffer_size);
    ring->push(buffer_ptr, buffer_size);

    return buffer_size;
}

bool Player::startStream()
{
    ca->start();
    progress_timer->start();
    return true;
}

bool Player::stopStream()
{
    progress_timer->stop();
    ca->stop();
    return true;
}

void Player::updateState(Player::State s)
{
    qDebug() << this << "updateState(): state changed to" << s;
    state = s;
    emit stateUpdated(state);
}

void Player::loadFile()
{
    PlayListItem *i = playlist->getCurrent();
    if (i) {
        try {
            file = new AVFile();
            file->setSamplerate(_sample_rate);
            file->setChannels(_channels);
            file->connectOutput(this);
            file->open(i->getUrlString().toLocal8Bit().constData());
            file_future = QtConcurrent::run([this](){
                file->decode();
                delete file;
                file = 0;
            });
            file_future_watcher.setFuture(file_future);
        } catch (AVException &e) {
            qWarning() << this << "loadFile(): unable to open file: " << e.what();
            if (file)
                delete file;
            return;
        }

        if (state == Player::PAUSE) {
            startStream();
            updateState(Player::PLAY);
        }
        emit itemUpdated(i);
    }
}

void Player::ejectFile()
{
    if (file) {
        file->abort();

        QFutureWatcher<void> eject_future_watcher;
        auto eject_future = QtConcurrent::run([this](){
            av_sample_t *buffer = new av_sample_t[4096];
            while (state != Player::PLAY && file) {
                qDebug() << this << "ejectFile(): pumping samples";
                pull(buffer, 4096);
            }
            delete [] buffer ;
        });
        eject_future_watcher.setFuture(eject_future);
        file_future.waitForFinished();
        eject_future_watcher.waitForFinished();
    }
}

void Player::seekTo(float p)
{
    qDebug() << this << "seekTo()" << p;
    if (file)
        file->seekToPercent(p);
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
        if (playlist->next()) {
            loadFile();
            startStream();
            updateState(Player::PLAY);
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
    emit itemUpdated(nullptr);
}

void Player::next()
{
    qDebug() << this << "next()";
    if (state == Player::STOP)
        return;

    if (playlist->next()) {
        ejectFile();
        loadFile();
    } else {
        stop();
    }
}

void Player::prev()
{
    qDebug() << this << "prev()";
    if (state == Player::STOP)
        return;

    if (playlist->prev()) {
        ejectFile();
        loadFile();
    } else {
        stop();
    }
}

void Player::onTrackEnd()
{
    qDebug() << this << "onTrackEnd()";
    if (state == Player::STOP)
        return;

    if (playlist->next()) {
        loadFile();
    } else {
        stop();
    }
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
