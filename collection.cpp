#include "collection.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include "playlistitem.h"

Collection::Collection(QObject *parent) :
    QObject(parent)
{
}

Collection::~Collection()
{
}

bool Collection::openDB()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir p(path);
    if (!p.exists()) {
        p.mkpath(path);
    }
    path.append("/collection.ejdb");
    // open here
    return true;
}

bool Collection::closeDB()
{
    return true;
}

void Collection::sync(PlayListItem */*i*/)
{
}

quint32 Collection::getCollectionSize()
{
    return 0;
}
