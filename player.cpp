#include "player.h"

#include "playlistmodel.h"
#include "playlistitem.h"

#include "avexception.h"
#include "avfile.h"
#include "avsplitter.h"
#include "avhistogram.h"
#include "avspectrogram.h"
#include "avspectrum.h"
#include "qcoreaudio.h"

#include <QtConcurrentRun>
#include <QSemaphore>
#include <QSettings>
#include <QImage>
#include <QPainter>
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
    ca(new QCoreAudio(this)), playlist(0), file(0), file_future(), file_future_watcher(),
    histogram_future(), histogram_future_watcher(),
    ring(0), ring_semaphor(0), ring_size(0),
    state(Player::STOP), cnt(0)
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

    cnt += buffer_size;
    if (cnt > _sample_rate / 4) {
        cnt = 0;
        emit progressUpdated(file->getPositionInPercents());
        emit timeComboUpdated(
            formatTime(file->getPositionInSeconds()) + " [" +
            formatTime(file->getDurationInSeconds()) +"]"
        );
    }

    return buffer_size;
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
    qDebug() << this << "updateState()" << "state changed to:" << s;
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
            file->open(i->getUrl().toLocal8Bit().constData());
            file_future = QtConcurrent::run([this](){
                file->decode();
                delete file;
                file = 0;
            });
            file_future_watcher.setFuture(file_future);

            histogram_future = QtConcurrent::run([this]() {
                emit histogramUpdated(0);
                emit histogramUpdated(analyze());
            });
            histogram_future_watcher.setFuture(histogram_future);

        } catch (AVException &e) {
            qWarning() << this << "loadFile()" << "unable to open file: " << e.what();
            if (file)
                delete file;
            return;
        }

        if (state == Player::PAUSE) {
            startStream();
            updateState(Player::PLAY);
        }
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
        histogram_future.waitForFinished();
        eject_future_watcher.waitForFinished();

        emit histogramUpdated(0);
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
    emit histogramUpdated(0);
    emit progressUpdated(-1);
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

QImage *Player::analyze()
{
    try {
        PlayListItem *i = playlist->getCurrent();
        if (!i->isLocalFile())
            return 0;

        AVFile file;
        AVSplitter splitter;
        AVHistogram histogram(4096);
        AVSpectrum spectrum(4096, AVSpectrum::BlackmanHarris);

        file.connectOutput(&splitter);
        splitter.connectOutput(&histogram);
        splitter.connectOutput(&spectrum);

        file.setChannels(1);
        file.setSamplerate(44100);
        file.open(i->getUrl().toLocal8Bit().constData());
        file.decode();

        std::deque<float> *hist=histogram.getData();
        std::deque<float> *spec=spectrum.getData();

        QImage *pic = new QImage(hist->size()/4, 160, QImage::Format_ARGB32);
        QPainter painter(pic);

        pic->fill(QColor(0,0,0,0));

        int x=0;
        while (true) {
            x++;
            if (!hist->size() || !spec->size())
                break;

            // color section
            float r, g, b;
            r = spec->front()*0.5; spec->pop_front();
            g = spec->front(); spec->pop_front();
            b = spec->front()*5.0; spec->pop_front();

            float maximum = r;
            if (g > maximum) {
                maximum = g;
            }
            if (b > maximum) {
                maximum = b;
            }

            if (maximum) {
                r = r / maximum * 180 + 30;
                g = g / maximum * 180 + 30;
                b = b / maximum * 180 + 30;
            } else {
                r = g = b = 0;
            }

            painter.setPen(QColor(r,g,b,64));

            // histogram section
            float pos_peak, neg_peak, pos_rms, neg_rms;
            pos_peak = hist->front()*80; hist->pop_front();
            neg_peak = hist->front()*80; hist->pop_front();

            painter.drawLine(x,80+pos_peak,x,80-neg_peak);

            painter.setPen(QColor(r,g,b));
            pos_rms  = hist->front()*80; hist->pop_front();
            neg_rms  = hist->front()*80; hist->pop_front();
            painter.drawLine(x,80+pos_rms,x,80-neg_rms);
        }
        return pic;
    } catch (AVException e) {
        qDebug() << "failed to create histogram" << e.what();
    }
    return new QImage(1, 1, QImage::Format_ARGB32);
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
