#include "src/journalproxymodel.h"

JournalProxyModel::JournalProxyModel(JournalModel *docModel, QObject *parent)
    : QSortFilterProxyModel(parent), model_{docModel}
{
    setSourceModel(model_);
}

Qt::ItemFlags JournalProxyModel::flags(const QModelIndex &ix) const
{
    return QSortFilterProxyModel::flags(ix) & ~Qt::EditRole;
}
