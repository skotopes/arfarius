#ifndef COLLECTION_H
#define COLLECTION_H

#include <QObject>

struct EJDB;
struct EJCOLL;

class PlayListItem;

class Collection : public QObject
{
    Q_OBJECT
public:
    explicit Collection(QObject *parent = 0);
    virtual ~Collection();

    bool openDB();
    bool closeDB();

    void sync(PlayListItem *i);

    quint32 getCollectionSize();

private:
    EJDB *db;
    EJCOLL *tracks;

signals:

public slots:

};

#endif // COLLECTION_H
