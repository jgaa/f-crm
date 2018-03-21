
#include <QDateEdit>
#include "src/documentproxymodel.h"

DocumentProxyModel::DocumentProxyModel(DocumentsModel *docModel, QObject *parent)
    : QSortFilterProxyModel(parent), model_{docModel}
{
    setSourceModel(model_);
}

QVariant DocumentProxyModel::data(const QModelIndex &ix, int role) const
{
    static const int h_type = model_->fieldIndex("type");
    static const int h_added_date = model_->fieldIndex("added_date");
    static const int h_cls = model_->fieldIndex("cls");
    static const int h_direction = model_->fieldIndex("direction");
    static const int h_entity = model_->fieldIndex("entity");
    static const int h_file_date = model_->fieldIndex("file_date");

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (ix.column() == h_added_date || ix.column() == h_file_date) {
            const auto when = model_->data(ix, Qt::DisplayRole).toDateTime();
            return when.date();
        }

        // TODO: Map person, intent, action to name

        if (ix.column() == h_type) {
            return Document::typeName(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
        }

        if (ix.column() == h_cls) {
            return Document::className(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
        }

        if (ix.column() == h_direction) {
            return Document::directionName(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
        }

        if (ix.column() == h_entity) {
            return Document::entityName(std::max(0, model_->data(ix, Qt::DisplayRole).toInt()));
        }

    }

    return model_->data(ix, role);
}


Qt::ItemFlags DocumentProxyModel::flags(const QModelIndex &ix) const
{
    static const int h_name = model_->fieldIndex("name");

    if (ix.isValid()) {
        if ((ix.column() == h_name)) {
            return QSortFilterProxyModel::flags(ix);
        }
    }


    return QSortFilterProxyModel::flags(ix) & ~Qt::EditRole;
}
