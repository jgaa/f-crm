#include "src/intentproxymodel.h"
#include <QDateEdit>


IntentProxyModel::IntentProxyModel(IntentsModel *docModel, QObject *parent)
    : QSortFilterProxyModel(parent), model_{docModel}
{
    setSourceModel(model_);
}

QVariant IntentProxyModel::data(const QModelIndex &ix, int role) const
{
    static const int h_state = model_->fieldIndex("state");
    static const int h_type = model_->fieldIndex("type");
    static const int h_abstract = model_->fieldIndex("abstract");

    if (ix.isValid()) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {

            if (ix.column() == h_type) {
                return GetIntentTypeName(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_state) {
                return GetIntentStateName(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
            }

        } else if (role == Qt::DecorationRole) {

            if (ix.column() == h_abstract) {
                const auto cix = index(ix.row(), h_type, {});
                return GetIntentTypeIcon(std::max(0, model_->data(cix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_type) {
                return GetIntentTypeIcon(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_state) {
                return GetIntentStateIcon(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
            }
        }
    }

    return model_->data(ix, role);
}


Qt::ItemFlags IntentProxyModel::flags(const QModelIndex &ix) const
{
    static const int h_abstract = model_->fieldIndex("abstract");

    if (ix.isValid()) {
        if ((ix.column() == h_abstract)) {
            return QSortFilterProxyModel::flags(ix);
        }
    }

    return QSortFilterProxyModel::flags(ix) & ~Qt::EditRole;
}
