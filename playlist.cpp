#include "playlist.h"

PlayList::PlayList(QObject *parent) :
    QAbstractTableModel(parent)
{
//    QSqlTableModel *model = new QSqlTableModel(parentObject, database);
//    model->setTable("employee");
//    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
//    model->select();
//    model->setHeaderData(0, Qt::Horizontal, tr("Name"));
//    model->setHeaderData(1, Qt::Horizontal, tr("Salary"));

}

int PlayList::rowCount(const QModelIndex & /*parent*/) const
{
   return 2;
}

int PlayList::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant PlayList::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
       return QString("Row%1, Column%2")
                   .arg(index.row() + 1)
                   .arg(index.column() +1);
    }
    return QVariant();
}
