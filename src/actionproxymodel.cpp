#include "src/actionproxymodel.h"
#include "channel.h"
#include "contact.h"
#include <QDateEdit>


ActionProxyModel::ActionProxyModel(ActionsModel *docModel, QObject *parent)
    : QSortFilterProxyModel(parent), model_{docModel}
{
    setSourceModel(model_);
}

QVariant ActionProxyModel::data(const QModelIndex &ix, int role) const
{
    //static const int h_name = model_->fieldIndex("name");
    static const int h_state = model_->fieldIndex("state");
    static const int h_type = model_->fieldIndex("type");
    static const int h_channel_type = model_->fieldIndex("channel_type");
    static const int h_person = model_->fieldIndex("person");
    static const int h_start_date = model_->fieldIndex("start_date");

    if (ix.isValid()) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            if (ix.column() == h_type) {
                return GetActionTypeName(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_state) {
                return GetActionStateName(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_person) {
                const auto id = model_->data(ix, Qt::DisplayRole).toInt();
                if (id > 0) {
                    QSqlQuery query(QStringLiteral("select name from contact where id = %1")
                                    .arg(id));
                    if (query.next()) {
                        return query.value(0).toString();
                    }
                }
            }
        } else if (role == Qt::DecorationRole) {
            if (ix.column() == h_start_date) {
                const auto cix = index(ix.row(), h_type, {});
                const int type = model_->data(cix, Qt::DisplayRole).toInt();
                if (type == static_cast<int>(ActionType::CHANNEL)) {
                    const auto cix = index(ix.row(), h_channel_type, {});
                    const int ctype = model_->data(cix, Qt::DisplayRole).toInt();
                    return GetChannelStatusIcon(ctype);
                }

                return GetActionTypeIcon(std::max(0, type));
            }

            if (ix.column() == h_state) {
                return GetActionStateIcon(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_type) {
                return GetActionTypeIcon(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_channel_type) {
                return GetChannelStatusIcon(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
            }
        }
    }

    return model_->data(ix, role);
}


Qt::ItemFlags ActionProxyModel::flags(const QModelIndex &ix) const
{
    static const int h_name = model_->fieldIndex("name");

    if (ix.isValid()) {
        if ((ix.column() == h_name)) {
            return QSortFilterProxyModel::flags(ix);
        }
    }

    return QSortFilterProxyModel::flags(ix) & ~Qt::EditRole;
}
