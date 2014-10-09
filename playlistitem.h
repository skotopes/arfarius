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
    bool isLocalFile();

    QString getUrlHash();
    QString getUrlString();
    QString getUrlStringLocal();

    static int getColumnsCount();
    static QString getColumnName(int col);
    QString getColumn(int col);
    void setColumn(int col, QString value);

    void ensureHistogram();

    bool hasHistogram();
    QString getHistogramDataPath();
    QImage * getHistogrammImage(size_t width, size_t height);

    static bool ensurePath();

private:
    QUrl    source;
    QString artist;
    QString title;
    QString album;
    int     duration;

    void readTags();
    void writeTags();
    void analyze();

signals:
    void histogramUpdated();
};

#endif // PLAYLISTITEM_H
