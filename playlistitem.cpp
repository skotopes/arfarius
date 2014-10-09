#include "playlistitem.h"

#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/tag.h>

#include "avexception.h"
#include "avsplitter.h"
#include "avhistogram.h"
#include "avspectrum.h"
#include "avfile.h"

#include <QCryptographicHash>
#include <QStandardPaths>
#include <QtConcurrent>
#include <QDataStream>
#include <QTextCodec>
#include <QFileInfo>
#include <QPainter>
#include <QImage>
#include <QDebug>
#include <QFile>

/*
 * Helpers
 */

QString formatTime(int time)
{
    int h,m,s;

    s = time % 60;
    m = time / 60 % 60;
    h = time / 60 / 60;

    if (h > 0)
        return QString("%1:%2:%3").arg(h, 2).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    else
        return QString("%1:%2").arg(m, 2).arg(s, 2, 10, QChar('0'));
}

QString toQstring(TagLib::String str) {
    if (str.isLatin1()) {
        QTextCodec* codec = QTextCodec::codecForName("cp1251");
        return codec->toUnicode(str.toCString());
    } else {
        return QString::fromUtf8(str.toCString(true));
    }
}


TagLib::String toString(QString str) {
    return TagLib::String(str.toStdString(), TagLib::String::UTF8);
}

/*
 * PlayListItem
 */

PlayListItem::PlayListItem(QUrl s) :
    QObject(), source(s), artist(), title(), album(), duration(-1)
{
}

PlayListItem::~PlayListItem()
{
}

bool PlayListItem::isValid()
{
    try {
        AVFile f;
        f.open(getUrlString().toLocal8Bit().constData());
        duration = f.getDurationInSeconds();
    } catch (AVException &e) {
        qDebug() << "PlayListItem::isValid() says NOO to "<< source << "because:" << e.what();
        return false;
    }

    if (source.isLocalFile()) {
        readTags();
    }

    return true;
}

bool PlayListItem::isLocalFile()
{
    return source.isLocalFile();
}

QString PlayListItem::getUrlHash()
{
    return QCryptographicHash::hash(getUrlStringLocal().toLocal8Bit(), QCryptographicHash::Sha3_512).toHex();
}

QString PlayListItem::getUrlString() {
    if (source.isLocalFile()) {
        return source.toLocalFile();
    } else {
        return source.toString();
    }
}

QString PlayListItem::getUrlStringLocal() {
    return source.toLocalFile();
}

int PlayListItem::getColumnsCount()
{
    return 4;
}

QString PlayListItem::getColumnName(int col)
{
    switch (col) {
    case 0:
        return "Artist";
    case 1:
        return "Title";
    case 2:
        return "Album";
    case 3:
        return "Time";
    default:
        return "UNKNOWN";
    }
}

QString PlayListItem::getColumn(int col)
{
    switch (col) {
    case 0:
        return artist;
    case 1:
        return title;
    case 2:
        return album;
    case 3:
        return formatTime(duration);
    default:
        return "UNKNOWN";
    }
}

void PlayListItem::setColumn(int col, QString value)
{
    bool w = true;
    switch (col) {
    case 0:
        artist = value;
        break;
    case 1:
        title = value;
        break;
    case 2:
        album = value;
        break;
    default:
        w = false;
        break;
    }

    if (w)
        writeTags();
}

bool PlayListItem::hasHistogram()
{
    QFile data_file(getHistogramDataPath());
    if (data_file.size())
        return true;

    return false;
}

void PlayListItem::ensureHistogram()
{
    if (!hasHistogram()) {
        QtConcurrent::run([&]{ auto hack = this; analyze(); hack = nullptr; });
    }
}

QString PlayListItem::getHistogramDataPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/histogram/" + getUrlHash();
}

QImage * PlayListItem::getHistogrammImage(size_t width, size_t height)
{
    qDebug() << this << "generating histogram image";
    if (!isLocalFile())
        return nullptr;

    QFile data_file(getHistogramDataPath());
    if (!data_file.open(QIODevice::ReadOnly)) {
        qDebug() << this << "Unable to open histogram data file";
        return nullptr;
    }

    qDebug() << this << "Requested image Width:" << width << "Height" << height;
    size_t blocks = data_file.size() / 7 / sizeof(float);
    if (!blocks) {
        return nullptr;
    } else if (blocks < width) {
        width = blocks;
    } else {
        size_t t_blocks = blocks;
        while (t_blocks > width*2) {
            t_blocks /= 2;
        }
        width = t_blocks;
    }
    qDebug() << this << "Final image Width:" << width << "Height" << height;

    size_t blocks_per_pixel = blocks / width;

    QDataStream data(&data_file);
    data.setFloatingPointPrecision(QDataStream::SinglePrecision);
    QImage *pic = new QImage(width, height, QImage::Format_ARGB32);
    pic->fill(QColor(0,0,0,0));

    QPainter painter(pic);

    size_t x=0, p=0;
    float hh = height/2;
    float pos_peak, neg_peak, pos_rms, neg_rms, r, g, b;
    float pos_peak_avg=0, neg_peak_avg=0, pos_rms_avg=0, neg_rms_avg=0, r_avg=0, g_avg=0, b_avg=0;
    while (true) {
        if (data.atEnd()) {
            qDebug() << this << "Unexpected end" << data_file.pos() << p << blocks_per_pixel;
            break;
        }
        data >> pos_peak >> neg_peak >> pos_rms >> neg_rms;
        data >> r >> g >> b;
        // Gain
        r = r*0.5;
        b = b*5.0;
        // find maximum
        float maximum = r;
        if (g > maximum) {
            maximum = g;
        }
        if (b > maximum) {
            maximum = b;
        }
        // Rescaling
        if (maximum) {
            r = r / maximum * 180 + 30;
            g = g / maximum * 180 + 30;
            b = b / maximum * 180 + 30;
        } else {
            r = g = b = 0;
        }
        // Summ data
        pos_peak_avg += pos_peak; neg_peak_avg += neg_peak;
        pos_rms_avg  += pos_rms;  neg_rms_avg  += neg_rms;
        r_avg += r; g_avg += g; b_avg += b;
        // Commit data to image
        if (++p == blocks_per_pixel) {
            pos_peak_avg /= blocks_per_pixel; neg_peak_avg /= blocks_per_pixel;
            pos_rms_avg  /= blocks_per_pixel; neg_rms_avg  /= blocks_per_pixel;
            r_avg /= blocks_per_pixel; g_avg /= blocks_per_pixel; b_avg /= blocks_per_pixel;
            // Peaks
            painter.setPen(QColor(r_avg, g_avg, b_avg, 64));
            painter.drawLine(
                x, hh + pos_peak_avg * hh,
                x, hh - neg_peak_avg * hh
            );
            // RMS
            painter.setPen(QColor(r_avg, g_avg, b_avg));
            painter.drawLine(
                x, hh + pos_rms_avg * hh,
                x, hh - neg_rms_avg * hh
            );
            // Break the circle
            if (++x == width)
                break;
            // zero
            p = 0;
            pos_peak_avg = neg_peak_avg = pos_rms_avg = neg_rms_avg = r_avg = g_avg = b_avg = 0;
        }
    }

    return pic;
}

void PlayListItem::readTags()
{
    qDebug() << this << "reading tags";
    TagLib::FileRef f(getUrlStringLocal().toLocal8Bit().constData());
    TagLib::Tag *t = f.tag();
    if (t && !t->isEmpty() && (!t->artist().isEmpty() && !t->title().isEmpty())) {
        artist = toQstring(t->artist());
        title = toQstring(t->title());
        album = toQstring(t->album());
    } else {
        QFileInfo fileInf(source.toLocalFile());
        title = fileInf.completeBaseName();
    }
}


void PlayListItem::writeTags()
{
    qDebug() << this << "writing tags";
    TagLib::FileRef f(getUrlStringLocal().toLocal8Bit().constData());
    if (!f.isNull()) {
        TagLib::Tag *t = f.tag();

        t->setArtist(toString(artist));
        t->setTitle(toString(title));
        t->setAlbum(toString(album));

        f.save();
    } else {
        qDebug() << this << "can not write tags into" << getUrlStringLocal();
    }
}

void PlayListItem::analyze()
{
    qDebug() << this << "analyzing";
    try {
        if (!isLocalFile())
            return;

        QFile data_file(getHistogramDataPath());
        if (!data_file.open(QIODevice::WriteOnly)) {
            qDebug() << this << "Unable to open histogram data file for write:" << getHistogramDataPath();
            return;
        }

        QDataStream data(&data_file);
        data.setFloatingPointPrecision(QDataStream::SinglePrecision);

        size_t h_count=0, s_count=0;

        AVFile      file;
        AVSplitter	splitter;
        AVHistogram histogram(8192);
        AVSpectrum  spectrum(8192, AVSpectrum::BlackmanHarris);

        histogram.dataCallback = [&](float p_p, float n_p, float p_r, float n_r) {
            data << p_p << n_p << p_r << n_r;
            h_count++;
        };

        spectrum.dataCallback = [&](float r, float g, float b) {
            data << r << g << b;
            s_count++;
        };

        file.connectOutput(&splitter);
        splitter.connectOutput(&histogram);
        splitter.connectOutput(&spectrum);

        file.setChannels(1);
        file.setSamplerate(22050);
        file.open(getUrlString().toLocal8Bit().constData());
        file.decode();

        Q_ASSERT(h_count == s_count);

        data_file.close();
        qDebug() << this << "histogram ready";
    } catch (AVException e) {
        qDebug() << this << "failed to create histogram" << e.what();
    }

    emit histogramUpdated();
}

bool PlayListItem::ensurePath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/histogram/";
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << "Creating histogram storage" << path;
        QDir::root().mkpath(path);
    }

    return true;
}
