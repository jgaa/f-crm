
#include <set>
#include <QSqlQuery>
#include <QSqlError>
#include <QRunnable>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlField>
#include <QDateTime>

#include "src/documentsmodel.h"
#include "src/strategy.h"
#include "src/intent.h"
#include "document.h"
#include "journalmodel.h"

using namespace std;


DocumentsModel::DocumentsModel(QSettings &settings, QObject *parent, QSqlDatabase db)
    : QSqlTableModel{parent, std::move(db)}
    , settings_{settings}
{
    setTable("document");
    setEditStrategy(QSqlTableModel::OnFieldChange);

    h_id_ = fieldIndex("id");
    h_contact_ = fieldIndex("contact");
    h_person_ = fieldIndex("person");
    h_intent_ = fieldIndex("intent");
    h_activity_ = fieldIndex("activity");
    h_type_ = fieldIndex("type");
    h_cls_ = fieldIndex("cls");
    h_direction_ = fieldIndex("direction");
    h_entity_ = fieldIndex("entity");
    h_name_ = fieldIndex("name");
    h_notes_ = fieldIndex("notes");
    h_added_date_ = fieldIndex("added_date");
    h_file_date_ = fieldIndex("file_date");
    h_location_ = fieldIndex("location");
    h_content_ = fieldIndex("content");

    Q_ASSERT(h_id_ >= 0
             && h_contact_ > 0
             && h_person_ > 0
             && h_intent_ > 0
             && h_activity_ > 0
             && h_type_ > 0
             && h_cls_ > 0
             && h_direction_ > 0
             && h_entity_ > 0
             && h_name_ > 0
             && h_notes_ > 0
             && h_added_date_ > 0
             && h_file_date_ > 0
             && h_location_ > 0
             && h_content_ > 0
             );


    setSort(h_added_date_, Qt::AscendingOrder);
    setFilter("id = -1"); // Filter everything away
}

void DocumentsModel::setContact(int id)
{
    setFilter(QStringLiteral("contact = %1").arg(id));
    select();
}

QSqlRecord DocumentsModel::getRecord(int contact,
                                     Document::Type type,
                                     Document::Class cls,
                                     Document::Direction direction,
                                     Document::Entity entity,
                                     int person, int intent, int action)
{
    Q_ASSERT(contact > 0);

    auto rec = record();
    rec.setValue(h_added_date_, QDateTime::currentDateTime());
    rec.setValue(h_type_, static_cast<int>(type));
    rec.setValue(h_cls_, static_cast<int>(cls));
    rec.setValue(h_direction_, static_cast<int>(direction));
    rec.setValue(h_entity_, static_cast<int>(entity));

    rec.setValue(h_contact_, contact);

    if (person > 0) rec.setValue(h_person_, person);
    if (intent > 0) rec.setValue(h_intent_, intent);
    if (action > 0) rec.setValue(h_activity_, action);

    Q_ASSERT(entity == Document::Entity::PERSON ? (person > 0) : true);
    Q_ASSERT(entity == Document::Entity::INTENT ? (intent > 0) : true);
    Q_ASSERT(entity == Document::Entity::ACTION ? (action > 0) : true);

    return rec;
}

void DocumentsModel::removeDocuments(const QModelIndexList &indexes)
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);

    set<int> rows;
    for(const auto& ix : indexes) {
        rows.insert(ix.row());
    }

    for(const int row : rows) {
        const auto rec = record(row);
        if (!removeRow(row, {})) {
            qWarning() << "Failed to remove row " << row << ": "
                       << lastError().text();
        }

        JournalModel::instance().addEntry(JournalModel::Type::DELETED_DOCUMENT,
                                    QStringLiteral("Deleted document: %1")
                                    .arg(rec.value("name").toString()),
                                    rec.value("contact").toInt(),
                                    rec.value("person").toInt(),
                                    rec.value("intent").toInt(),
                                    rec.value("action").toInt(),
                                    rec.value("id").toInt());
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new contact (submitAll): "
                   << lastError().text();
    }
}

void DocumentsModel::addDocument(const QSqlRecord &origRec)
{
    QSqlRecord rec = origRec;
    fix(rec);
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);

    if (!insertRecord(-1, rec)) {
        qWarning() << "Failed to add new document (insertRecord): "
                   << lastError().text();
        return;
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new document (submitAll): "
                   << lastError().text();
        return;
    }

    qDebug() << "Created new document";

    JournalModel::instance().addEntry(JournalModel::Type::ADD_DOCUMENT,
                                QStringLiteral("Added document: %1").arg(origRec.value("name").toString()),
                                origRec.value("contact").toInt(),
                                origRec.value("person").toInt(),
                                origRec.value("intent").toInt(),
                                origRec.value("action").toInt(),
                                query().lastInsertId().toInt());
}

void DocumentsModel::updateDocument(const int row, const QSqlRecord &rec)
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);

    if (!setRecord(row, rec)) {
        qWarning() << "Failed to update document (setRecord): "
                   << lastError().text();
        return;
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new document (submitAll): "
                   << lastError().text();
        return;
    }

    JournalModel::instance().addEntry(JournalModel::Type::UPDATED_DOCUMENT,
                                QStringLiteral("Updated document: %1")
                                .arg(rec.value("name").toString()),
                                rec.value("contact").toInt(),
                                rec.value("person").toInt(),
                                rec.value("intent").toInt(),
                                rec.value("action").toInt(),
                                rec.value("id").toInt());
}


QVariant DocumentsModel::data(const QModelIndex &ix, int role) const
{
    if (ix.isValid()) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            if (ix.column() == h_added_date_ || ix.column() == h_file_date_) {
                const auto when = QDateTime::fromTime_t(
                            QSqlTableModel::data(ix, Qt::DisplayRole).toUInt());
                return when;
            }

//            // TODO: Map person, intent, action to name

//            if (ix.column() == h_type_) {
//                return Document::typeName(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
//            }

//            if (ix.column() == h_cls_) {
//                return Document::className(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
//            }

//            if (ix.column() == h_direction_) {
//                return Document::directionName(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
//            }

//            if (ix.column() == h_entity_) {
//                return Document::entityName(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
//            }

        } else if (role == Qt::DecorationRole) {
            if (ix.column() == h_added_date_) {
                return Document::typeIcon(std::max(0,
                                                   QSqlTableModel::data(
                                                       index(ix.row(), h_type_, {}),
                                                       Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_type_) {
                return Document::typeIcon(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_cls_) {
                return Document::classIcon(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_direction_) {
                return Document::directionIcon(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_entity_) {
                return Document::entityIcon(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }
        }
    }

    return QSqlTableModel::data(ix, role);
}

QVariant DocumentsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {

        if (section == h_added_date_) {
            return QStringLiteral("Date");
        }

        auto name = QSqlTableModel::headerData(section, orientation, role).toString();
        name[0] = name[0].toUpper();
        return name;
    }
    return QSqlTableModel::headerData(section, orientation, role);
}


bool DocumentsModel::updateRowInTable(int row, const QSqlRecord &values)
{
    QSqlRecord rec{values};
    fix(rec);

    return QSqlTableModel::updateRowInTable(row, rec);
}

void DocumentsModel::fix(QSqlRecord &rec)
{
    if (rec.value("person").toInt() <= 0) {
        rec.setNull("person");
    }
    if (rec.value("intent").toInt() <= 0) {
        rec.setNull("intent");
    }
    if (rec.value("activity").toInt() <= 0) {
        rec.setNull("activity");
    }

    auto value = rec.value(h_added_date_);
    if (value.type() == QVariant::DateTime) {
        rec.setValue(h_added_date_, value.toDateTime().toTime_t());
    }

    value = rec.value(h_file_date_);
    if (value.type() == QVariant::DateTime && !value.isNull()) {
        rec.setValue(h_file_date_, value.toDateTime().toTime_t());
    }
}

