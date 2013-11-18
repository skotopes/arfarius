#include "player.h"

#include "playlistmodel.h"
#include "playlistitem.h"

#include "avexception.h"
#include "avfile.h"
#include "avsplitter.h"
#include "avhistogram.h"
#include "avspectrogram.h"
#include "RtError.h"

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
    dac(0), playlist(0), file(0), file_future(), file_future_watcher(),
    buffer(new MemRing<av_sample_t>(16384)), buffer_semaphor(new QSemaphore(16384)),
    state(Player::STOP), cnt(0)
{
    qRegisterMetaType<Player::State>("Player::State");
    connect(&file_future_watcher, SIGNAL(finished()), this, SLOT(onTrackEnd()));
    openDAC();
}

Player::~Player()
{
    if (file) {
        ejectFile();
    }

    if (dac) {
        stopStream();
        closeDAC();
    }

    if (buffer) {
        delete buffer;
        buffer = 0;
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
    size_t ret = buffer->pull(buffer_ptr, buffer_size);
    buffer_semaphor->release(ret);
    return ret;
}

size_t Player::push(float *buffer_ptr, size_t buffer_size)
{
    size_t ret;
    if (buffer_semaphor->tryAcquire(buffer_size, 500)) {
        ret = buffer->push(buffer_ptr, buffer_size);
    } else {
        ret = buffer->push(buffer_ptr, buffer_size);
        buffer_semaphor->acquire(ret);
    }

    cnt += buffer_size;
    if (cnt > _sample_rate / 4) {
        cnt = 0;
        emit progressUpdated(file->getPositionInPercents());
        emit timeComboUpdated(
            formatTime(file->getPositionInSeconds()) + " [" +
            formatTime(file->getDurationInSeconds()) +"]"
        );
    }
    return ret;
}

int Player::callback(void *outputBuffer, void *inputBuffer,
             unsigned int nBufferFrames, double streamTime,
             RtAudioStreamStatus status, void *userData)
{
    Q_UNUSED(inputBuffer)
    Q_UNUSED(streamTime)
    Q_UNUSED(status)

    Player *me = reinterpret_cast<Player *>(userData);
    float *buffer = reinterpret_cast<float *>(outputBuffer);

    nBufferFrames *= me->_channels;
    size_t ret = me->pull(buffer, nBufferFrames);
    if (ret != nBufferFrames) {
        buffer += ret;
        memset(buffer, 0, (nBufferFrames - ret) * sizeof(av_sample_t));
        qWarning() << me << "callback()" << "Buffer underrun";
    }

    return 0;
}

void Player::openDAC()
{
    QSettings settings;
    _sample_rate = settings.value("dac/samplerate", 44100).toInt();
    _channels = settings.value("dac/channels", 2).toInt();

    dac = new RtAudio();

    if (dac->getDeviceCount() < 1) {
        qWarning() << this << "openStream(): No output found";
        return;
    }

    auto device_id = dac->getDefaultOutputDevice();
    RtAudio::StreamParameters parameters;
    parameters.deviceId = device_id;
    parameters.nChannels = _channels;
    parameters.firstChannel = 0;

    unsigned int buffer_frames = 256;

    try {
        dac->openStream( &parameters, 0, RTAUDIO_FLOAT32,
                        _sample_rate, &buffer_frames,
                        &callback, this );
    } catch ( RtError& e ) {
        qWarning() << this << "openStream() RtError:" << e.what();
    }
}

void Player::startStream()
{
    try {
        dac->startStream();
    } catch ( RtError& e ) {
        qWarning() << this << this << "startStream() RtError:" << e.what();
    }
}

void Player::stopStream()
{
    try {
        if (dac->isStreamRunning())
            dac->stopStream();
    } catch (RtError& e) {
        qWarning() << this << "stopStream() RtError:" << e.what();
    }
}

void Player::closeDAC()
{
    try {
        if (dac->isStreamOpen())
            dac->closeStream();
    } catch (RtError& e) {
        qWarning() << this << "stopStream() RtError:" << e.what();
    }
    delete dac; dac=0;
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
            QtConcurrent::run([this]() {analyze();});
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
        if (state != PLAY) {
            startStream();
            updateState(Player::PLAY);
        }
        file_future.waitForFinished();
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
    updateState(Player::STOP);
    emit timeComboUpdated("[>(^_^)<]");
    emit progressUpdated(-1);
    emit histogramUpdated(0);
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

void Player::analyze()
{
    PlayListItem *i = playlist->getCurrent();
    if (!i->isLocalFile())
        return;

    AVFile file;
    AVSplitter splitter;
    AVHistogram histogram(2048);
    AVSpectrogram spectrogram(2048);

    file.connectOutput(&splitter);
    splitter.connectOutput(&histogram);
    splitter.connectOutput(&spectrogram);

    file.setChannels(1);
    file.setSamplerate(44100);
    file.open(i->getUrl().toLocal8Bit().constData());
    file.decode();

    std::deque<float> *hist=histogram.getData();
    std::deque<float> *spec=spectrogram.getData();

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
        r = spec->front()/2; spec->pop_front();
        g = spec->front(); spec->pop_front();
        b = spec->front()*10; spec->pop_front();

        float maximum = r;
        if (g > maximum) {
            maximum = g;
        }
        if (b > maximum) {
            maximum = b;
        }

        if (maximum) {
            r = r / maximum * 220;
            g = g / maximum * 200;
            b = b / maximum * 255;
        } else {
            r = g = b = 0;
        }

        painter.setPen(QColor(r,g,b,128));

        // histogram section
        float pos_peak, neg_peak, pos_rms, neg_rms;
        pos_peak = hist->front()*80; hist->pop_front();
        neg_peak = hist->front()*80; hist->pop_front();

        painter.drawLine(x,80+pos_peak,x,80-neg_peak);

        painter.setPen(QColor(r,g,b));
        pos_rms  = hist->front()*80; hist->pop_front();
        neg_rms  = hist->front()*80; hist->pop_front();
        painter.drawLine(x,80+pos_rms,x,80-neg_rms);
//        qDebug() << this << "analyze()" << r << g << b << pos_peak << neg_peak << pos_rms << neg_rms;
    }

    emit histogramUpdated(pic);
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
