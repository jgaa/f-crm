#include "src/contactproxymodel.h"
#include <QDateEdit>

ContactProxyModel::ContactProxyModel(ContactsModel *docModel, QObject *parent)
    : QSortFilterProxyModel(parent), model_{docModel}
{
    setSourceModel(model_);
}

QVariant ContactProxyModel::data(const QModelIndex &ix, int role) const
{
    static const int h_status = model_->fieldIndex("status");

    if (role == Qt::DisplayRole || role == Qt::EditRole) {

        if (ix.column() == h_status) {
            return GetContactStatusName(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
        }
    }

    return model_->data(ix, role);
}


Qt::ItemFlags ContactProxyModel::flags(const QModelIndex &ix) const
{
    static const int h_name = model_->fieldIndex("name");

    if (ix.isValid()) {
        if ((ix.column() == h_name)) {
            return QSortFilterProxyModel::flags(ix);
        }
    }


    return QSortFilterProxyModel::flags(ix) & ~Qt::EditRole;
}
