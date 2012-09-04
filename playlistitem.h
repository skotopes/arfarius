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

    void populateSource();
    bool isVlaid();

    QUrl    source;
    QString artist;
    QString name;
};

#endif // PLAYLISTITEM_H
