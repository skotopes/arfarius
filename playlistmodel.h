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

    virtual Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool removeRows(int row, int count, const QModelIndex &parent);

    void clickedItem(const QModelIndex &index = QModelIndex());

signals:
    void itemUpdated(PlayListItem*);

public slots:
    void nextItem();
    void prevItem();

    void appendUrl(QUrl url);
    void appendUrls(QList<QUrl> urls);
    void appendItems(QList<PlayListItem *> new_items);

private:
    QList<PlayListItem *> items;
    int current;

    QList<PlayListItem *> urlToItems(QUrl url);
    QList<PlayListItem *> urlsToItems(QList<QUrl> urls);
    QList<QUrl> m3uToUrls(QUrl url);

};

#endif // PLAYLISTMODEL_H
