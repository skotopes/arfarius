#include "playlistitem.h"
#include "avfile.h"

#include <QDebug>

PlayListItem::PlayListItem(QUrl s) :
    QObject(), source(s), artist(), name()
{
}

PlayListItem::~PlayListItem()
{
}

void PlayListItem::populateSource()
{
    AVFile f;
    f.open(source.toString().toStdString());
    qDebug() << "f is audio:" << f.isAudio();
}

bool PlayListItem::isVlaid()
{
    return true;
}
