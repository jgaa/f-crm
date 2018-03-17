
#include <set>

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

using namespace std;

ContactsModel::ContactsModel(QSettings& settings, QObject *parent, QSqlDatabase db)
    : QSqlTableModel{parent, std::move(db)}
    , settings_{settings}
{
    setTable("contact");
    setEditStrategy(QSqlTableModel::OnFieldChange);

    h_id_ = fieldIndex("id");
    h_contact_ = fieldIndex("contact");
    h_created_date_ = fieldIndex("created_date");
    h_last_activity_date_ = fieldIndex("last_activity_date");
    h_name_ = fieldIndex("name");
    h_first_name_ = fieldIndex("first_name");
    h_last_name_ = fieldIndex("last_name");
    h_middle_name_ = fieldIndex("middle_name");
    h_gender_ = fieldIndex("gender");
    h_type_ = fieldIndex("type");
    h_status_ = fieldIndex("status");
    h_notes_ = fieldIndex("notes");
    h_stars_ = fieldIndex("stars");
    h_favorite_ = fieldIndex("favourite");
    h_address1_ = fieldIndex("address1");
    h_address2_ = fieldIndex("address2");
    h_city_ = fieldIndex("city");
    h_postcode_ = fieldIndex("postcode");
    h_country_ = fieldIndex("country");

    Q_ASSERT(h_contact_ > 0
            && h_created_date_ > 0
            && h_last_activity_date_ > 0
            && h_name_ > 0
            && h_first_name_ > 0
            && h_last_name_ > 0
            && h_middle_name_ > 0
            && h_gender_ > 0
            && h_type_ > 0
            && h_status_ > 0
            && h_notes_ > 0
            && h_stars_ > 0
            && h_favorite_ > 0
            && h_address1_ > 0
            && h_address2_ > 0
            && h_city_ > 0
            && h_postcode_ > 0
            && h_country_ > 0
    );

    setSort(h_name_, Qt::AscendingOrder);
    setNameFilter({});
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
//    if (!internal_edit_) {
//        if (ix.isValid()) {
//            if ((ix.column() != h_name_)
//               && (ix.column() != h_notes_)) {
//                return QSqlTableModel::flags(ix) & ~Qt::EditRole;
//            }
//        }
//    }

    return QSqlTableModel::flags(ix);
}

int ContactsModel::getContactId(const QModelIndex &ix) const
{
    Q_ASSERT(ix.isValid());
    return data(index(ix.row(), property("id_col").toInt(), {}),
                Qt::DisplayRole).toInt();
}

void ContactsModel::setNameFilter(const QString &filter)
{
    QString parent_filter;
    if (parent_) {
        parent_filter = QStringLiteral("contact = %1").arg(parent_);
    } else {
        parent_filter = "contact is NULL";
    }

    if (filter.isEmpty()) {
        setFilter(parent_filter);
    } else {
        // We have to escape certain characters
        QString escaped = filter;
        escaped.replace("'", "''");
        escaped.replace("_", "\\_");
        escaped.replace("%", "\\%");
        QString full_filter;
        full_filter = QStringLiteral("%1 and name like '%%%2%%' ESCAPE '\\'").arg(parent_filter).arg(escaped);
        setFilter(full_filter);
    }
}

void ContactsModel::setParent(int contact)
{
    parent_ = contact;
    setNameFilter({});
}

void ContactsModel::removeContacts(const QModelIndexList &indexes)
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);

    const auto internal_edit_save = internal_edit_;
    internal_edit_ = true;
    auto release_edit = make_release([this, internal_edit_save]{
        internal_edit_ = internal_edit_save;
    });

    set<int> rows;
    for(const auto& ix : indexes) {
        rows.insert(ix.row());
    }

    for(const int row : rows) {
        if (!removeRow(row, {})) {
            qWarning() << "Failed to remove row " << row << ": "
                       << lastError().text();
        }
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new contact (submitAll): "
                   << lastError().text();
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

    const auto now = static_cast<uint>(time(nullptr));
    rec.setValue(h_name_, QStringLiteral(""));
    rec.setValue(h_created_date_, now);
    rec.setValue(h_last_activity_date_, now);

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
