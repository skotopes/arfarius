#include "player.h"

#include "arfariusapplication.h"
#include "playlistitem.h"

#include "avexception.h"
#include "avfile.h"
#include "qcoreaudio.h"

#include <QtConcurrentRun>
#include <QSemaphore>
#include <QSettings>
#include <QTimer>
#include <QDebug>

namespace {

QString formatTime(size_t time) {
    int h, m, s;

    s = time % 60;
    m = time / 60 % 60;
    h = time / 60 / 60;

    if(h > 0)
        return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    else
        return QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));
}

} // anonymous namespace

Player::Player(QObject* parent)
    : QObject(parent)
    , ca(new QCoreAudio(this))
    , file(nullptr)
    , file_future()
    , eject_future()
    , ring(nullptr)
    , ring_semaphor(nullptr)
    , ring_size(0)
    , samples_elapsed(0)
    , state(Player::STOP)
    , quiet(false) {
    qRegisterMetaType<Player::State>("Player::State");

    ca->open();
    ca->connectInput(this);
    _sample_rate = ca->getDeviceSampleRate();
    _channels = 2;

    ring_size = _sample_rate / 4;
    ring = new MemRing<av_sample_t>(ring_size);
    ring_semaphor = new QSemaphore(ring_size);
}

Player::~Player() {
    if(file) {
        ejectFile();
    }

    if(ca) {
        stopStream();
    }

    if(ring) {
        delete ring;
        ring = nullptr;
    }

    if(ring_semaphor) {
        delete ring_semaphor;
        ring_semaphor = nullptr;
    }
}

const char* Player::getName() {
    return "Player";
}

size_t Player::pull(float* buffer_ptr, size_t buffer_size) {
    size_t ret = ring->pull(buffer_ptr, buffer_size);
    ring_semaphor->release(ret);

    return ret;
}

size_t Player::push(float* buffer_ptr, size_t buffer_size) {
    size_t buffer_size_orig = buffer_size;
    while(buffer_size > 0) {
        size_t granula_size = buffer_size;
        if(granula_size > ring_size / 8) {
            granula_size = ring_size / 8;
        }
        ring_semaphor->acquire(granula_size);
        ring->push(buffer_ptr, granula_size);
        buffer_size -= granula_size;
        buffer_ptr += granula_size;
    }

    samples_elapsed.fetch_add(buffer_size_orig, std::memory_order_relaxed);
    if(samples_elapsed.load(std::memory_order_relaxed) > (_sample_rate * _channels / 8)) {
        samples_elapsed.store(0, std::memory_order_relaxed);
        QMetaObject::invokeMethod(this, "onProgressTimer", Qt::QueuedConnection);
    }

    return buffer_size_orig;
}

bool Player::startStream() {
    ca->start();
    return true;
}

bool Player::stopStream() {
    ca->stop();
    return true;
}

void Player::updateState(Player::State s) {
    qDebug() << this << "updateState(): state changed to" << s;
    state.store(s, std::memory_order_relaxed);
    emit stateUpdated(state.load(std::memory_order_relaxed));
}

void Player::ejectFile() {
    qDebug() << this << "ejectFile()";
    if(!file) return;

    file->cancelDecoding();

    // Drain the ring buffer so the decoder can exit cleanly.
    // The pump must run regardless of current state.
    eject_future = QtConcurrent::run(arfariusApp->getDecoderThreadPool(), [this]() {
        av_sample_t* buffer = new av_sample_t[4096];
        while(file) {
            pull(buffer, 4096);
        }
        delete[] buffer;
    });
    file_future.waitForFinished();
    eject_future.waitForFinished();

    delete file;
    file = nullptr;
}

void Player::updateItem(PlayListItem* item) {
    qDebug() << this << "updateItem()" << item;
    if(item) {
        if(file) {
            quiet.store(true, std::memory_order_release);
        }
        ejectFile();

        file = item->getAVFile();
        if(file == nullptr) {
            emit trackEnded();
            return;
        }

        file->setSamplerate(_sample_rate);
        file->setChannels(_channels);
        file->connectOutput(this);
        file_future = QtConcurrent::run(arfariusApp->getDecoderThreadPool(), [this]() {
            file->decode();
            delete file;
            file = nullptr;
            if(quiet.load(std::memory_order_acquire)) {
                quiet.store(false, std::memory_order_release);
            } else {
                QMetaObject::invokeMethod(this, "trackEnded", Qt::QueuedConnection);
            }
        });

        if(state.load(std::memory_order_relaxed) != Player::PLAY) {
            startStream();
            updateState(Player::PLAY);
        }
    } else {
        stop();
    }
}

void Player::seekToPercent(float p) {
    qDebug() << this << "seekTo()" << p;
    if(file) file->seekToPercent(p);
}

void Player::seekForward(float seconds) {
    qDebug() << this << "seekForward" << seconds;
    if(file) file->seekForward(seconds);
}

void Player::seekBackward(float seconds) {
    qDebug() << this << "seekBackward" << seconds;
    if(file) file->seekBackward(seconds);
}

void Player::play() {
    qDebug() << this << "play()";
    if(state.load(std::memory_order_relaxed) == Player::PAUSE) {
        startStream();
        updateState(Player::PLAY);
    } else if(state.load(std::memory_order_relaxed) == Player::STOP) {
        startStream();
        updateState(Player::PLAY);
        if(!file) {
            emit trackEnded();
        }
    }
}

void Player::pause() {
    qDebug() << this << "pause()";
    if(state.load(std::memory_order_relaxed) == Player::PLAY) {
        stopStream();
        updateState(Player::PAUSE);
    }
}

void Player::playPause() {
    qDebug() << this << "playPause()";
    if(state.load(std::memory_order_relaxed) == Player::PLAY) {
        stopStream();
        updateState(Player::PAUSE);
    } else if(state.load(std::memory_order_relaxed) == Player::PAUSE) {
        startStream();
        updateState(Player::PLAY);
    } else if(state.load(std::memory_order_relaxed) == Player::STOP) {
        startStream();
        updateState(Player::PLAY);
        if(!file) {
            emit trackEnded();
        }
    }
}

void Player::stop() {
    qDebug() << this << "stop()";
    if(state.load(std::memory_order_relaxed) == Player::STOP) return;

    ejectFile();
    stopStream();

    ring->reset();
    int e = ring_semaphor->available();
    if(e < (int)ring_size) {
        ring_semaphor->release(ring_size - e);
    }
    updateState(Player::STOP);
    emit timeComboUpdated("=(-_-)=");
    emit progressUpdated(-1);
}

void Player::onProgressTimer() {
    if(!file) return;
    float file_duration = file->getDurationInSeconds();
    float file_samples = file->getDurationInSamples();
    float file_position = file->getPositionInPercents();
    float ring_offset = (float)ring->readSpace() / _channels / file_samples;

    float percent = file_position - ring_offset;
    float position = percent * file_duration;

    emit progressUpdated(percent);
    emit timeComboUpdated(formatTime(position) + " [" + formatTime(file_duration) + "]");
}
