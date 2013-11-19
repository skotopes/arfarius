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

    QString getUrl();
    QString getUrlLocalFile();
    bool isLocalFile();
    void setPos(int p);
    int getPos();

    QString getColumn(int col);
    void setColumn(int col, QString value);

    static int getColumnsCount();
    static QString getColumnName(int col);

private:
    int     pos;
    QUrl    source;
    QString artist;
    QString title;
    QString album;
    int     time;
    bool    hasTag;

    void readTags();
    void writeTags();

    QString formatTime();
};

#endif // PLAYLISTITEM_H
