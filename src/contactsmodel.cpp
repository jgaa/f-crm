
#include <QSqlQuery>
#include <QSqlError>
#include <QRunnable>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlField>
#include <QDateTime>
#include <QUuid>

#include "src/contactsmodel.h"
#include "src/strategy.h"
#include "src/release.h"

ContactsModel::ContactsModel(QSettings& settings, QObject *parent, QSqlDatabase db)
    : QSqlTableModel{parent, std::move(db)}
    , settings_{settings}
{

    Q_UNUSED(parent);
    Q_UNUSED(db);

    setTable("contact");
    setEditStrategy(QSqlTableModel::OnFieldChange);

    h_id_ = fieldIndex("id");
    h_name_ = fieldIndex("name");
    h_first_name_ = fieldIndex("first_name");
    h_last_name_ = fieldIndex("last_name");
    h_middle_name = fieldIndex("middle_name");
    h_gender = fieldIndex("gender");
    h_type_ = fieldIndex("type");
    h_status_ = fieldIndex("status");
    h_notes_ = fieldIndex("notes");
    h_stars_ = fieldIndex("stars");
    h_favorite_ = fieldIndex("favourite");

    setSort(h_name_, Qt::AscendingOrder);

}


QVariant ContactsModel::data(const QModelIndex &ix, int role) const
{
    return QSqlTableModel::data(ix, role);
}

QVariant ContactsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        auto name = QSqlTableModel::headerData(section, orientation, role).toString();
        name[0] = name[0].toUpper();
        return name;
    }
    return QSqlTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ContactsModel::flags(const QModelIndex &ix) const
{
    if (!internal_edit_) {
        if (ix.isValid()) {
            if (ix.column() != h_name_) {
                return QSqlTableModel::flags(ix) & ~Qt::EditRole;
            }
        }
    }

    return QSqlTableModel::flags(ix);
}

void ContactsModel::setNameFilter(const QString &filter)
{
    if (filter.isEmpty()) {
        setFilter({});
    } else {
        // We have to escape certain characters
        QString escaped = filter;
        escaped.replace("'", "''");
        escaped.replace("_", "\\_");
        escaped.replace("%", "\\%");
        QString full_filter = QStringLiteral("name like '%%%1%%' ESCAPE '\\'").arg(escaped);
        setFilter(full_filter);
    }

}

QModelIndex ContactsModel::createContact()
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);
    auto rec = database().record(tableName());

//    qDebug() << "Using database "
//             << database().databaseName()
//             << " with driver " << database().driverName()
//             << " and connection " << database().connectionName()
//             << " with tables " << this->database().tables()
//             << ". Open status: " << this->database().isOpen();

    rec.setValue(h_name_, QStringLiteral(""));

//    for(int i = 0; i < rec.count(); ++i) {
//        qDebug() << "# " << i << " " << rec.fieldName(i)
//                 << " " << rec.value(i).typeName()
//                 << " : " << (rec.isNull(i) ? QStringLiteral("NULL") : rec.value(i).toString());
//    }

    const auto internal_edit_save = internal_edit_;
    internal_edit_ = true;
    auto release_edit = make_release([this, internal_edit_save]{
        internal_edit_ = internal_edit_save;
    });

    if (!insertRecord(0, rec)) {
        qWarning() << "Failed to add new contact (insertRecord): "
                   << lastError().text();
        return {};
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new contact (submitAll): "
                   << lastError().text();
        return {};
    }

    qDebug() << "Created new contact";

    return index(0, h_name_, {}); // Assume that we insterted at end in the model
}
