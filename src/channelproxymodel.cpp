#include "src/channelproxymodel.h"

ChannelProxyModel::ChannelProxyModel(ChannelsModel *docModel, QObject *parent)
    : QSortFilterProxyModel(parent), model_{docModel}
{
    setSourceModel(model_);
}

QVariant ChannelProxyModel::data(const QModelIndex &ix, int role) const
{
    static const int h_value = model_->fieldIndex("value");
    static const int h_verified = model_->fieldIndex("verified");
    static const int h_type = model_->fieldIndex("type");

    static const QIcon check_icon{":/res/icons/check.svg"};

    if (ix.isValid()) {
        if (role == Qt::DecorationRole && ix.column() == h_value) {
            auto nix = index(ix.row(), h_type, {});
            int status = model_->data(nix, Qt::DisplayRole).toInt();
            return GetChannelStatusIcon(status);
        }

        if (ix.column() == h_verified) {
            if (role == Qt::DecorationRole) {
                auto nix = index(ix.row(), h_verified, {});
                if (model_->data(nix, Qt::DisplayRole).toBool()) {
                    return check_icon;
                }
            }

            if (role == Qt::DisplayRole) {
                return {};
            }
        }
    }

    return model_->data(ix, role);
}


Qt::ItemFlags ChannelProxyModel::flags(const QModelIndex &ix) const
{
    static const int h_value = model_->fieldIndex("value");

    if (ix.isValid()) {
        if ((ix.column() == h_value)) {
            return QSortFilterProxyModel::flags(ix);
        }
    }

    return QSortFilterProxyModel::flags(ix) & ~Qt::EditRole;
}
