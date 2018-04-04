
#include <set>
#include <array>

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
#include "src/journalmodel.h"

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
    h_region_ = fieldIndex("region");
    h_state_ = fieldIndex("city");
    h_country_ = fieldIndex("country");

    Q_ASSERT(h_contact_ > 0
            && h_created_date_ > 0
            && h_last_activity_date_ > 0
            && h_name_ > 0
            && h_gender_ > 0
            && h_type_ > 0
            && h_status_ > 0
            && h_notes_ > 0
            && h_stars_ > 0
            && h_favorite_ > 0
            && h_address1_ > 0
            && h_address2_ > 0
            && h_city_ > 0
            && h_region_ > 0
            && h_city_ > 0
            && h_state_ > 0
            && h_country_ > 0
    );

    setSort(h_name_, Qt::AscendingOrder);
    setNameFilter({});
}


QVariant ContactsModel::data(const QModelIndex &ix, int role) const
{
    if (ix.isValid()) {
        if (role == Qt::DecorationRole) {
            if (ix.column() == h_name_) {
                const auto cix = index(ix.row(), h_type_, {});
                return GetContactTypeIcon(std::max(0, QSqlTableModel::data(cix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_status_) {
                return GetContactStatusIcon(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_type_) {
                return GetContactTypeIcon(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_favorite_) {
                return getFavoriteIcon(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_stars_) {
                return getStars(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }
        }

    }
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

        const auto rec = record(row);
        const auto id = rec.value("id").toInt();
        const auto parent = rec.value("contact").toInt();
        const bool is_company = rec.value(h_type_).toInt() == static_cast<int>(ContactType::CORPORATION);

        JournalModel::instance().addEntry(JournalModel::Type::DELETED_SOMETHING,
                                          QStringLiteral("Deleted %1 #%2 %3")
                                          .arg(is_company ? "Contact" : "Person")
                                          .arg(rec.value(h_id_).toInt())
                                          .arg(rec.value(h_name_).toString()),
                                          parent ? parent : id,
                                          parent ? id : 0);

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

void ContactsModel::addPerson(const QSqlRecord &rec)
{
    QSqlRecord my_rec{rec};
    insertContact(my_rec);
    select();
}

void ContactsModel::toggleFavoriteStatus(const int row)
{
    Q_ASSERT(row >= 0);

    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);
    const auto ix = index(row, h_favorite_, {});
    const bool new_status = !data(ix, Qt::DisplayRole).toBool();
    if (!setData(ix, new_status)) {
        qWarning() << "Failed to set stars (setData): "
                   << lastError().text();
        return;
    }

    const auto id = data(index(row, h_id_, {}), Qt::DisplayRole).toInt();

    if (!submitAll()) {
        qWarning() << "Failed to update flag (submitAll): "
                   << lastError().text();
        return;
    }

    JournalModel::instance().addEntry(
                JournalModel::Type::UPDATED_CONTACT,
                new_status ? "Set the Favourite flag" : "Removed the Favourite flag",
                id);
}

void ContactsModel::setStars(const int row, const int stars)
{
    Q_ASSERT(row >= 0);
    Q_ASSERT(stars >= 0 && stars <= 5);

    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);

    const auto ix = index(row, h_stars_, {});
    if (!setData(ix, stars)) {
        qWarning() << "Failed to set stars (setData): "
                   << lastError().text();
        return;
    }
    if (!submitAll()) {
        qWarning() << "Failed to update flag (submitAll): "
                   << lastError().text();
        return;
    }
    const auto id = data(index(row, h_id_, {}), Qt::DisplayRole).toInt();

    JournalModel::instance().addEntry(
                JournalModel::Type::UPDATED_CONTACT,
                QStringLiteral("Set %1 stars")
                .arg(stars),
                id);
}

bool ContactsModel::insertContact(QSqlRecord &rec)
{
    const auto now = static_cast<uint>(time(nullptr));
    rec.setValue(h_created_date_, now);
    rec.setValue(h_last_activity_date_, now);

    if (!insertRecord(0, rec)) {
        qWarning() << "Failed to add new contact (insertRecord): "
                   << lastError().text();
        return false;
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new contact (submitAll): "
                   << lastError().text();
        return false;
    }

    const auto contact_id = query().lastInsertId().toInt();
    const auto what = parent_ ? "Person" : "Contact";
    const auto contact_type = rec.value(h_type_).toInt();

    const auto log_type = contact_type == static_cast<int>(ContactType::CORPORATION)
            ? JournalModel::Type::ADD_COMPANY
            : JournalModel::Type::ADD_PERSON;

    JournalModel::instance().addEntry(
                log_type,
                QStringLiteral("Added %1: %2").arg(what).arg(rec.value(h_name_).toString()),
                parent_ ? parent_ : contact_id,
                parent_ ? contact_id : 0);

    qDebug() << QStringLiteral("Created new %1 #").arg(what) << contact_id;
    return true;
}

const QIcon& ContactsModel::getFavoriteIcon(const bool enable)
{
    static const QIcon fav(":/res/icons/favourite.svg");
    static const QIcon not_fav(":/res/icons/not_favourite.svg");

    return enable ? fav : not_fav;
}

const QIcon &ContactsModel::getStars(const int stars)
{
    static const std::array<QIcon,6> icons = {{
        QIcon(":/res/icons/0star.svg"),
        QIcon(":/res/icons/1star.svg"),
        QIcon(":/res/icons/2star.svg"),
        QIcon(":/res/icons/3star.svg"),
        QIcon(":/res/icons/4star.svg"),
        QIcon(":/res/icons/5star.svg"),
    }};

    return icons.at(static_cast<size_t>(stars));
}

QModelIndex ContactsModel::createContact(const ContactType type)
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);
    auto rec = record();

    rec.setValue(h_name_, QStringLiteral(""));
    rec.setValue(h_type_, static_cast<int>(type));

    if (!insertContact(rec)) {
        return {};
    }

    return index(0, h_name_, {}); // Assume that we insterted at end in the model
}


