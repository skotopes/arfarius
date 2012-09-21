#include "playlistitem.h"

#include "avexception.h"
#include "avfile.h"

#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/tag.h>

#include <QFileInfo>
#include <QDebug>

PlayListItem::PlayListItem(QUrl s) :
    QObject(), pos(0), source(s), artist(), title()
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
    } catch (AVException &e) {
        qDebug() << "PlayListItem::isValid() says NOO to "<< source << "because:" << e.what();
        return false;
    }

    if (source.isLocalFile()) {
        TagLib::FileRef f(getUrlLocalFile().toLocal8Bit().constData());
        TagLib::Tag *t = f.tag();
        if (t && !t->isEmpty()) {
            artist = t->artist().toCString();
            title = t->title().toCString();
        } else {
            QFileInfo fileInf(source.toLocalFile());
            title = fileInf.fileName();
        }
    }

    return true;
}
