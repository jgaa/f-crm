#include "src/logproxymodel.h"

LogProxyModel::LogProxyModel(LogModel *docModel, QObject *parent)
    : QSortFilterProxyModel(parent), model_{docModel}
{
    setSourceModel(model_);
}

Qt::ItemFlags LogProxyModel::flags(const QModelIndex &ix) const
{
    return QSortFilterProxyModel::flags(ix) & ~Qt::EditRole;
}
