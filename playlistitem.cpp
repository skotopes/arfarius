#include "playlistitem.h"

PlayListItem::PlayListItem(QUrl s) :
    QObject(), source(s), artist(), name()
{
}

PlayListItem::~PlayListItem()
{
}
