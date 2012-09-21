#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QAbstractTableModel>
#include <QList>
#include <QUrl>

class PlayListItem;

class PlayList : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PlayList(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    PlayListItem * getCurrent();

    bool next();
    bool prev();

signals:

public slots:
    void appendUrls(QList<QUrl> urls);

private:
    QList<PlayListItem *> items;
    int current;
};

#endif // PLAYLIST_H
