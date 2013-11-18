#include "playlistitem.h"

#include "avexception.h"
#include "avfile.h"

#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/tag.h>

#include <QFileInfo>
#include <QDebug>

PlayListItem::PlayListItem(QUrl s) :
    QObject(), pos(-1), source(s), artist(), title(), album(), time(-1), hasTag(false)
{
}

PlayListItem::~PlayListItem()
{
}

bool PlayListItem::isValid()
{
    try {
        AVFile f;
        f.open(getUrl().toLocal8Bit().constData());
        time = f.getDurationInSeconds();
    } catch (AVException &e) {
        qDebug() << "PlayListItem::isValid() says NOO to "<< source << "because:" << e.what();
        return false;
    }

    if (source.isLocalFile()) {
        readTags();
    }

    return true;
}

QString PlayListItem::getColumn(int col)
{
    switch (col) {
    case 0:
        return QString("%1").arg(pos);
    case 1:
        return artist;
    case 2:
        return title;
    case 3:
        return album;
    case 4:
        return formatTime();
    default:
        return "UNKNOWN";
    }
}

void PlayListItem::setColumn(int col, QString value)
{
    bool w = true;
    switch (col) {
    case 1:
        artist = value;
        break;
    case 2:
        title = value;
        break;
    case 3:
        album = value;
        break;
    default:
        w = false;
        break;
    }

    if (w)
        writeTags();
}

int PlayListItem::getColumnsCount()
{
    return 5;
}

QString PlayListItem::getColumnName(int col)
{
    switch (col) {
    case 0:
        return "#";
    case 1:
        return "Artist";
    case 2:
        return "Title";
    case 3:
        return "Album";
    case 4:
        return "Time";
    default:
        return "UNKNOWN";
    }
}

void PlayListItem::readTags()
{
    TagLib::FileRef f(getUrlLocalFile().toLocal8Bit().constData());
    TagLib::Tag *t = f.tag();
    if (t && !t->isEmpty() && (t->artist().length()>0 && t->title().length() >0)) {
        artist = t->artist().toCString();
        title = t->title().toCString();
        album = t->album().toCString();
    } else {
        QFileInfo fileInf(source.toLocalFile());
        title = fileInf.fileName();
    }
}

void PlayListItem::writeTags()
{
    TagLib::FileRef f(getUrlLocalFile().toLocal8Bit().constData());
    TagLib::Tag *t = f.tag();

    t->setArtist(artist.toStdString());
    t->setTitle(title.toStdString());
    t->setAlbum(album.toStdString());

    f.save();
}

QString PlayListItem::formatTime()
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
