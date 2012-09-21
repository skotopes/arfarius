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
    inline QString getUrl() { return source.toString(); }
    inline QString getArtist() { return artist; }
    inline QString getName() { return name; }

private:
    QUrl    source;
    QString artist;
    QString name;
    bool    hasTag;

};

#endif // PLAYLISTITEM_H
