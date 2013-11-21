#include "collection.h"

#include <tcejdb/ejdb.h>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include "playlistitem.h"

Collection::Collection(QObject *parent) :
    QObject(parent), db(0), tracks(0)
{
}

Collection::~Collection()
{
    if (db) closeDB();
}

bool Collection::openDB()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir p(path);
    if (!p.exists()) {
        p.mkpath(path);
    }
    path.append("/collection.ejdb");

    db = ejdbnew();
    if (!db) {
        qWarning() << this << "openDB(): Unable to allocate db structure";
        return false;
    }

    if (!ejdbopen(db, path.toLocal8Bit().constData(), JBOREADER | JBOWRITER | JBOCREAT)) {
        qWarning() << this << "openDB(): Unable to open collection";
        return false;
    }

    tracks = ejdbcreatecoll(db, "tracks", NULL);

    if (!tracks) {
        qWarning() << this << "openDB(): unable to create tracks collection";
        return false;
    }

    return true;
}

bool Collection::closeDB()
{
    if (!db) {
        qWarning() << this << "closeDB(): db structure is not allocated";
        return false;
    }

    if (!ejdbclose(db)) {
        qWarning() << this << "closeDB(): can not close database, data damage possible";
        return false;
    }

    ejdbdel(db);
    db = 0;

    return true;
}

void Collection::sync(PlayListItem *i)
{
    // prepare query
    bson query_bson;
    bson_init_as_query(&query_bson);
    bson_append_string(&query_bson, "path", i->getUrl().toLocal8Bit().constData());
    bson_finish(&query_bson);

    uint32_t count;
    EJQ *query = ejdbcreatequery(db, &query_bson, NULL, 0, NULL);
    TCLIST *result = ejdbqryexecute(tracks, query, &count, 0, NULL);
    qDebug() << this << "Records found: " << count;

    for (int i = 0; i < TCLISTNUM(result); ++i) {
        void *bsdata = TCLISTVALPTR(result, i);
        bson_print_raw((char *)bsdata, 0);
    }

    tclistdel(result);
    ejdbquerydel(query);
    bson_destroy(&query_bson);
}

quint32 Collection::getCollectionSize()
{
    bson query_bson;
    bson_init_as_query(&query_bson);
    bson_finish(&query_bson);

    uint32_t count;
    EJQ *query = ejdbcreatequery(db, &query_bson, NULL, 0, NULL);
    ejdbqryexecute(tracks, query, &count, JBQRYCOUNT, NULL);

    ejdbquerydel(query);
    bson_destroy(&query_bson);

    return count;
}
