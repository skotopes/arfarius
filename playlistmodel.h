#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QUrl>

class PlayListItem;

class PlayListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PlayListModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool removeRows(int row, int count, const QModelIndex &parent);

    PlayListItem * getCurrent();

    bool next();
    bool prev();

signals:

public slots:
    void appendFile(QUrl);
    void appendDirectory(QUrl);
    void appendUrls(QList<QUrl> urls);
    void clear();

private:
    QList<PlayListItem *> items;
    int current;
};

#endif // PLAYLISTMODEL_H