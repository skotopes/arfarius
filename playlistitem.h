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
    inline QString getUrlLocalFile() { return source.toLocalFile(); }
    inline void setPos(int p) { pos = p; }
    inline int getPos() { return pos; }

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
    QString time;
    bool    hasTag;

    void readTags();
    void writeTags();

};

#endif // PLAYLISTITEM_H
