#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QAbstractTableModel>

class PlayList : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PlayList(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

signals:
    
public slots:
    
private:

};

#endif // PLAYLIST_H
