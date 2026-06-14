#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QUrl>

class PlayListItem;

class PlayListModel : public QAbstractTableModel
{
    friend class PlayListView;

    Q_OBJECT
public:
    explicit PlayListModel(QObject *parent = 0);

protected:
    // model base
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    // drag'n'drop
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual Qt::DropActions supportedDropActions() const;

    QStringList mimeTypes() const;
//    QMimeData *mimeData(const QModelIndexList &indexes) const;
//    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    // extra
    void clickedItem(const QModelIndex &index = QModelIndex());

signals:
    void itemUpdated(PlayListItem*);

public slots:
    void nextItem();
    void prevItem();

    void save(QString filename);
    void gather(QString path);
    void removeCurrent();

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
