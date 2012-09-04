#include "playlistitem.h"

PlayListItem::PlayListItem(QUrl s) :
    QObject(), source(s), artist(), name()
{
}

PlayListItem::~PlayListItem()
{
}

void PlayListItem::populateSource()
{
    // check tags and source playability
}

bool PlayListItem::isVlaid()
{
    return true;
}
