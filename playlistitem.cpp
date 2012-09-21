#include "playlistitem.h"
#include "avexception.h"
#include "avfile.h"

#include <QDebug>

PlayListItem::PlayListItem(QUrl s) :
    QObject(), source(s), artist(), name()
{
}

PlayListItem::~PlayListItem()
{
}

bool PlayListItem::isValid()
{
    try {
        AVFile f;
        f.open(getUrl().toAscii().constData());
    } catch (AVException &e) {
        qDebug() << "PlayListItem::isValid() says NOO to "<< source << "because:" << e.what();
        return false;
    }

    return true;
}

