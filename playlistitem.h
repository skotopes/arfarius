#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QObject>
#include <QUrl>
#include <QString>

class PlayListItem : public QObject
{
    Q_OBJECT
public:
    PlayListItem(QUrl s);
    virtual ~PlayListItem();

    bool isValid();

    inline void setPos(int p) { pos = p; }
    inline int getPos() { return pos; }
    inline QString getUrl() { return source.toString(); }
    inline QString getUrlLocalFile() { return source.toLocalFile(); }
    inline QString getUrlLast() { return source.toLocalFile(); }
    inline QString getArtist() { return artist; }
    inline QString getTitle() { return title; }

private:
    int     pos;
    QUrl    source;
    QString artist;
    QString title;
    bool    hasTag;

};

#endif // PLAYLISTITEM_H
