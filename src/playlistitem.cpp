#include "playlistitem.h"

#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/tag.h>

#include "arfariusapplication.h"
#include "avexception.h"
#include "avsplitter.h"
#include "avhistogram.h"
#include "avspectrum.h"
#include "avfile.h"

#include <QCryptographicHash>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QtConcurrent>
#include <QDataStream>
#include <QFileInfo>
#include <QPainter>
#include <QImage>
#include <QDebug>
#include <QFile>
#include <QDir>

/*
 * Helpers
 */

namespace {

QString formatTime(int time) {
    int h, m, s;

    s = time % 60;
    m = time / 60 % 60;
    h = time / 60 / 60;

    if(h > 0)
        return QString("%1:%2:%3").arg(h, 2).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    else
        return QString("%1:%2").arg(m, 2).arg(s, 2, 10, QChar('0'));
}

QString toQstring(TagLib::String str) {
    // TagLib::String::isLatin1() checks for ISO-8859-1 range.
    // cp1251 is a Windows encoding used for Russian/Cyrillic text.
    // If TagLib detected ISO-8859-1, use UTF-8 (the safe default for TagLib).
    // If TagLib detected UTF-8, use UTF-8.
    // cp1251 is only used as a fallback when TagLib cannot determine encoding.
    // TagLib::String::isLatin1() returns true for ISO-8859-1 and false for UTF-8.
    // For cp1251 (Cyrillic), TagLib typically stores as UTF-8, so isLatin1() would be false.
    // We use cp1251 only as a best-effort fallback for strings TagLib marked as Latin-1
    // but which contain extended characters that look like cp1251 bytes.
    if(str.isLatin1()) {
        auto toUtf16 = QStringDecoder("cp1251");
        return toUtf16(str.toCString(true));
    } else {
        return QString::fromUtf8(str.toCString(true));
    }
}

TagLib::String toString(QString str) {
    return TagLib::String(str.toStdString(), TagLib::String::UTF8);
}

} // anonymous namespace

/*
 * PlayListItem
 */

PlayListItem::PlayListItem(QUrl s)
    : QObject()
    , source(s)
    , artist()
    , title()
    , album()
    , duration(-1)
    , busy(false) {
}

PlayListItem::~PlayListItem() {
}

bool PlayListItem::isBusy() {
    return busy;
}

bool PlayListItem::isValid() {
    try {
        AVFile f;
        f.open(getUrlString().toLocal8Bit().constData());
        duration = f.getDurationInSeconds();
    } catch(AVException& e) {
        qDebug() << "PlayListItem::isValid() says NOO to " << source << "because:" << e.what();
        return false;
    }

    if(source.isLocalFile()) {
        readTags();
    }

    return true;
}

bool PlayListItem::isLocalFile() {
    return source.isLocalFile();
}

QUrl PlayListItem::getUrl() {
    return source;
}

void PlayListItem::setUrl(QUrl url) {
    source = url;
}

QString PlayListItem::getUrlHash() {
    QString value = getUrlStringLocal();
    if(value.isEmpty()) {
        value = source.toString();
    }
    return QCryptographicHash::hash(value.toLocal8Bit(), QCryptographicHash::Sha3_512).toHex();
}

QString PlayListItem::getUrlString() {
    QString url;
    if(source.isLocalFile()) {
        QStorageInfo storage_info(source.toLocalFile());
        auto fs_type = storage_info.fileSystemType();
        qDebug() << this << "getUrlString(): underlying fs is" << fs_type;
        if(fs_type == "smbfs") {
            qDebug() << this << "getUrlString(): using async io and caching";
            url = QString("async:cache:%1").arg(source.toLocalFile());
        } else {
            url = source.toLocalFile();
        }
    } else {
        url = source.toString();
    }

    return url;
}

QString PlayListItem::getUrlStringLocal() {
    return source.toLocalFile();
}

int PlayListItem::getColumnsCount() {
    return 4;
}

QString PlayListItem::getColumnName(int col) {
    switch(col) {
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

QString PlayListItem::getColumn(int col) {
    switch(col) {
    case 0:
        return getArtist();
    case 1:
        return getTitle();
    case 2:
        return getAlbum();
    case 3:
        return getFormattedDuration();
    default:
        return "UNKNOWN";
    }
}

QString PlayListItem::getArtist() {
    return artist;
}

QString PlayListItem::getTitle() {
    if(source.isLocalFile()) {
        if(artist.isEmpty() && title.isEmpty() && album.isEmpty()) {
            QFileInfo fileInf(source.toLocalFile());
            return fileInf.completeBaseName();
        } else {
            return title;
        }
    } else {
        return source.toString();
    }
}

QString PlayListItem::getAlbum() {
    return album;
}

QString PlayListItem::getFormattedDuration() {
    if(source.isLocalFile()) {
        return formatTime(duration);
    } else {
        return "??";
    }
}

void PlayListItem::setColumn(int col, QString value) {
    bool w = true;
    switch(col) {
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

    if(w) writeTags();
}

bool PlayListItem::hasHistogram() {
    QFile data_file(getHistogramDataPath());
    if(data_file.size()) return true;

    return false;
}

void PlayListItem::ensureHistogram() {
    if(!hasHistogram()) {
        busy = true;
        (void)QtConcurrent::run(arfariusApp->getAnalyzeThreadPool(), [this] {
            analyze();
            busy = false;
        });
    }
}

QString PlayListItem::getHistogramDataPath() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/histogram/" +
           getUrlHash();
}

QImage* PlayListItem::getHistogrammImage(size_t width, size_t height) {
    if(!isLocalFile()) return nullptr;

    QString dataPath = getHistogramDataPath();
    QFile data_file(dataPath);
    if(!data_file.open(QIODevice::ReadOnly)) {
        return nullptr;
    }

    size_t blocks = data_file.size() / 7 / sizeof(float);
    if(!blocks) {
        return nullptr;
    }

    // Clamp width to available data
    if(blocks < width) {
        width = blocks;
    } else {
        size_t t_blocks = blocks;
        while(t_blocks > width * 2) {
            t_blocks /= 2;
        }
        width = t_blocks;
    }

    if(width == 0 || height == 0) {
        return nullptr;
    }

    size_t blocks_per_pixel = blocks / width;
    // Guard against zero — should not happen after clamping, but be safe
    if(blocks_per_pixel == 0) {
        return nullptr;
    }

    QDataStream data(&data_file);
    data.setFloatingPointPrecision(QDataStream::SinglePrecision);

    QImage* pic = new QImage(width, height, QImage::Format_ARGB32);
    pic->fill(QColor(0, 0, 0, 0));

    QPainter painter(pic);

    size_t x = 0, p = 0;
    float hh = height / 2.0f;
    float pos_peak, neg_peak, pos_rms, neg_rms, r, g, b;
    float pos_peak_avg = 0, neg_peak_avg = 0, pos_rms_avg = 0, neg_rms_avg = 0, r_avg = 0,
          g_avg = 0, b_avg = 0;

    while(true) {
        if(data.atEnd()) {
            break;
        }
        data >> pos_peak >> neg_peak >> pos_rms >> neg_rms;
        data >> r >> g >> b;

        // Gain
        r = r * 0.5f;
        b = b * 5.0f;

        // find maximum
        float maximum = r;
        if(g > maximum) maximum = g;
        if(b > maximum) maximum = b;

        // Rescaling
        if(maximum > 0) {
            r = r / maximum * 180.0f + 30.0f;
            g = g / maximum * 180.0f + 30.0f;
            b = b / maximum * 180.0f + 30.0f;
        } else {
            r = g = b = 0;
        }

        // Summ data
        pos_peak_avg += pos_peak;
        neg_peak_avg += neg_peak;
        pos_rms_avg += pos_rms;
        neg_rms_avg += neg_rms;
        r_avg += r;
        g_avg += g;
        b_avg += b;

        // Commit data to image
        if(++p == blocks_per_pixel) {
            pos_peak_avg /= blocks_per_pixel;
            neg_peak_avg /= blocks_per_pixel;
            pos_rms_avg /= blocks_per_pixel;
            neg_rms_avg /= blocks_per_pixel;
            r_avg /= blocks_per_pixel;
            g_avg /= blocks_per_pixel;
            b_avg /= blocks_per_pixel;

            painter.setPen(QColor(
                qBound(0, qRound(r_avg / 2), 255),
                qBound(0, qRound(g_avg / 2), 255),
                qBound(0, qRound(b_avg / 2), 255)));
            painter.drawLine(x, qRound(hh + pos_peak_avg * hh), x, qRound(hh - neg_peak_avg * hh));

            painter.setPen(QColor(
                qBound(0, qRound(r_avg), 255),
                qBound(0, qRound(g_avg), 255),
                qBound(0, qRound(b_avg), 255)));
            painter.drawLine(x, qRound(hh + pos_rms_avg * hh), x, qRound(hh - neg_rms_avg * hh));

            if(++x == width) break;
            p = 0;
            pos_peak_avg = neg_peak_avg = pos_rms_avg = neg_rms_avg = 0;
            r_avg = g_avg = b_avg = 0;
        }
    }

    return pic;
}

void PlayListItem::readTags() {
    TagLib::FileRef f(getUrlStringLocal().toLocal8Bit().constData());
    TagLib::Tag* t = f.tag();
    if(t && !t->isEmpty()) {
        if(!t->artist().isEmpty()) artist = toQstring(t->artist());
        if(!t->title().isEmpty()) title = toQstring(t->title());
        if(!t->album().isEmpty()) album = toQstring(t->album());
    }
}

void PlayListItem::writeTags() {
    TagLib::FileRef f(getUrlStringLocal().toLocal8Bit().constData());
    if(!f.isNull()) {
        TagLib::Tag* t = f.tag();

        t->setArtist(toString(artist));
        t->setTitle(toString(title));
        t->setAlbum(toString(album));

        f.save();
    } else {
        qDebug() << this << "can not write tags into" << getUrlStringLocal();
    }
}

void PlayListItem::analyze() {
    try {
        if(!isLocalFile()) return;

        QFile data_file(getHistogramDataPath());
        if(!data_file.open(QIODevice::WriteOnly)) {
            qDebug() << this
                     << "Unable to open histogram data file for write:" << getHistogramDataPath();
            return;
        }

        QDataStream data(&data_file);
        data.setFloatingPointPrecision(QDataStream::SinglePrecision);

        size_t h_count = 0, s_count = 0;

        AVFile file;
        AVSplitter splitter;
        AVHistogram histogram(8192);
        AVSpectrum spectrum(8192, AVSpectrum::BlackmanHarris);

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

        data_file.close();

        if(h_count != s_count) {
            qWarning() << this << "histogram/spectrum count mismatch:" << h_count << s_count;
        }

        qDebug() << this << "histogram ready";
    } catch(const AVException& e) {
        qDebug() << this << "failed to create histogram" << e.what();
    }

    emit histogramUpdated();
}

AVFile* PlayListItem::getAVFile() {
    AVFile* file = nullptr;
    try {
        file = new AVFile;
        file->open(getUrlString().toLocal8Bit().constData());
    } catch(const AVException& e) {
        qDebug() << this << "Failed to open file because of" << e.what();
        delete file;
        file = nullptr;
    }

    return file;
}

bool PlayListItem::ensurePath() {
    QString path =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/histogram/";
    QDir dir(path);
    if(!dir.exists()) {
        QDir::root().mkpath(path);
    }

    return true;
}
