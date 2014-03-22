#include "collection.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include "playlistitem.h"

Collection::Collection(QObject *parent) :
    QObject(parent)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir p(path);

    if (!p.exists()) {
        p.mkpath(path);
    }

    path.append("/collection.ejdb");
    // open here

}

Collection::~Collection()
{
}

void Collection::syncPlayListItem(PlayListItem *i)
{
    Q_UNUSED(i);
}

quint32 Collection::getCollectionSize()
{
    return 0;
}
