#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QObject>
#include <QUrl>

class AVFile;

class PlayListItem : public QObject
{
    Q_OBJECT
public:
    PlayListItem(QUrl s);
    virtual ~PlayListItem();

    bool isValid();
    bool isLocalFile();

    QUrl getUrl();
    void setUrl(QUrl url);
    QString getUrlHash();
    QString getUrlString();
    QString getUrlStringLocal();

    // representation related methods
    static int getColumnsCount();
    static QString getColumnName(int col);
    QString getColumn(int col);
    void setColumn(int col, QString value);

    // histogramm related methods
    void ensureHistogram();
    bool hasHistogram();
    QString getHistogramDataPath();
    QImage * getHistogrammImage(size_t width, size_t height);

    // avfile related methods
    AVFile * getAVFile();
    static bool ensurePath();

private:
    QUrl    source;
    QString artist;
    QString title;
    QString album;
    int     duration;

    QString getArtist();
    QString getTitle();
    QString getAlbum();
    QString getFormattedDuration();

    void readTags();
    void writeTags();
    void analyze();

signals:
    void histogramUpdated();
};

#endif // PLAYLISTITEM_H
