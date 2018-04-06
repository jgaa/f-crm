#include <set>
#include <QSqlQuery>
#include <QSqlError>
#include <QRunnable>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlField>
#include <QDateTime>

#include "src/actionsmodel.h"
#include "src/strategy.h"
#include "src/intent.h"
#include "src/channel.h"
#include "src/utility.h"
#include "src/action.h"
#include "src/strategy.h"
#include "src/journalmodel.h"

using namespace std;

ActionsModel::ActionsModel(QSettings &settings, QObject *parent, QSqlDatabase db)
    : QSqlTableModel{parent, std::move(db)}
    , settings_{settings}
{
    setTable("action");
    setEditStrategy(QSqlTableModel::OnFieldChange);

    h_id_ = fieldIndex("id");
    h_sequence_ = fieldIndex("sequence");
    h_contact_ = fieldIndex("contact");
    h_intent_ = fieldIndex("intent");
    h_person_ = fieldIndex("person");
    h_state_ = fieldIndex("state");
    h_type_ = fieldIndex("type");
    h_channel_type_ = fieldIndex("channel_type");
    h_name_ = fieldIndex("name");
    h_created_date_ = fieldIndex("created_date");
    h_start_date_ = fieldIndex("start_date");
    h_due_date_ = fieldIndex("due_date");
    h_desired_outcome_ = fieldIndex("desired_outcome");
    h_notes_ = fieldIndex("notes");

    Q_ASSERT(h_sequence_ > 0
            && h_sequence_ > 0
            && h_contact_ > 0
            && h_intent_ > 0
            && h_person_ > 0
            && h_state_ > 0
            && h_type_ > 0
            && h_channel_type_ > 0
            && h_name_ > 0
            && h_created_date_ > 0
            && h_start_date_ > 0
            && h_due_date_ > 0
            && h_desired_outcome_ > 0
            && h_notes_ > 0
    );

    setSort(h_sequence_, Qt::AscendingOrder);
    setFilter("id = -1"); // Filter everything away
}

void ActionsModel::setContact(int id)
{
    contact_ = id;
    intent_ = 0;

    setFilter("id = -1");
    select();
}

void ActionsModel::setIntent(int id)
{
    intent_ = id;
    setFilter(QStringLiteral("contact = %1 and intent = %2").arg(contact_).arg(intent_));
    select();
}

void ActionsModel::removeActions(const QModelIndexList &indexes)
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

        JournalModel::instance().addEntry(JournalModel::Type::DELETE_ACTION,
                                    QStringLiteral("Deleted action: %1")
                                    .arg(rec.value("name").toString()),
                                    rec.value("contact").toInt(),
                                    rec.value("person").toInt(),
                                    0,
                                    rec.value("id").toInt());
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new contact (submitAll): "
                   << lastError().text();
    }
}

void ActionsModel::addAction(const QSqlRecord &origRec)
{
    QSqlRecord rec = origRec;
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);


//    for(int i = 0; i < rec.count(); ++i) {
//        qDebug() << "# " << i << " " << rec.fieldName(i)
//                 << " " << rec.value(i).typeName()
//                 << " : " << (rec.isNull(i) ? QStringLiteral("NULL") : rec.value(i).toString());
//    }


    if (rec.value(h_person_).toInt() <= 0) {
        rec.setNull(h_person_);
    }

    if (!insertRecord(-1, rec)) {
        qWarning() << "Failed to add new action (insertRecord): "
                   << lastError().text();
        return;
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new action (submitAll): "
                   << lastError().text();
        return;
    }

    qDebug() << "Created new intent";

    JournalModel::instance().addEntry(JournalModel::Type::ADD_ACTION,
                                QStringLiteral("Added action: %1").arg(origRec.value("name").toString()),
                                origRec.value("contact").toInt(),
                                origRec.value("person").toInt(),
                                0,
                                query().lastInsertId().toInt());
}

void ActionsModel::setCompleted(const QModelIndex& ix)
{
    const auto aix = index(ix.row(), h_state_, {});
    setData(aix, static_cast<int>(ActionState::DONE));
    openNextActions();
}

void ActionsModel::moveUp(const QModelIndex &ix)
{
    doMove(ix, -1);
}

void ActionsModel::moveDown(const QModelIndex &ix)
{
   doMove(ix, 1);
}

void ActionsModel::openNextActions()
{
    int prev_state = -1;
    for(int row = 0; row < rowCount(); ++row) {
        const auto state_ix = index(row, h_state_, {});
        const auto state = data(state_ix, Qt::DisplayRole).toInt();
        if (state == static_cast<int>(ActionState::WAITING)) {
            if (prev_state >= static_cast<int>(ActionState::DONE)) {
                qDebug() << "Releasing a new WAITING action";
                setData(state_ix, static_cast<int>(ActionState::OPEN));
                break;
            }
        }
        prev_state = state;
    }
}

void ActionsModel::updateState()
{
    QSqlQuery query(QStringLiteral("select state from intent where id=%1 ")
              .arg(intent_));
    if (query.next() && query.value(0).toInt() >= static_cast<int>(IntentState::PROGRESS)) {
        for(int i = 0; i < rowCount(); ++i) {
            const auto state = data(index(i, h_state_, {}), Qt::DisplayRole).toInt();
            if (state < static_cast<int>(ActionState::DONE)) {
                setData(index(i, h_state_), static_cast<int>(ActionState::CANCELLED));
            }
        }
    }
}

void ActionsModel::doMove(const QModelIndex &ix, const int offset)
{
    if (!ix.isValid()
            || ((ix.row() + offset) < 0)
            || ((ix.row() + offset) >= columnCount())) {
        return;
    }

    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);

    const auto other_ix = index(ix.row() + offset, h_sequence_, {});
    const auto curr_ix = index(ix.row(), h_sequence_, {});

    const auto other_seq = data(other_ix, Qt::DisplayRole);
    const auto curr_seq = data(curr_ix, Qt::DisplayRole);

    setData(other_ix, curr_seq);
    setData(curr_ix, other_seq);

    if (!submitAll()) {
        qWarning() << "Failed to swap actions: "
                   << lastError().text();
        return;
    }
}

QVariant ActionsModel::data(const QModelIndex &ix, int role) const
{
    if (ix.isValid()) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {

            if (ix.column() == h_created_date_
                    || ix.column() == h_start_date_
                    || ix.column() == h_due_date_) {
                const auto when = QDateTime::fromTime_t(
                            QSqlTableModel::data(ix, Qt::DisplayRole).toUInt());
                return when.date();
            }

        }
    }

    return QSqlTableModel::data(ix, role);
}

QVariant ActionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        auto name = QSqlTableModel::headerData(section, orientation, role).toString();
        name[0] = name[0].toUpper();
        return name.replace('_', ' ');
    }
    return QSqlTableModel::headerData(section, orientation, role);
}


bool ActionsModel::updateRowInTable(int row, const QSqlRecord &values)
{
    QSqlRecord rec{values};
    // Make sure we don't propagate a 0 value from the persons dropdown in the dilaog to the database
    if (values.value("person").toInt() <= 0) {
        rec.setNull("person");
    }

    // Check date and time fields.
    // QDataWidgetMapper save these as strings
    auto value = rec.value(h_start_date_);
    if (value.type() == QVariant::Date) {
        const auto date = rec.value(h_start_date_).toDate();
        qDebug() << "updateRowInTable: Start date is " << date;
        rec.setValue(h_start_date_, static_cast<uint>(ToTime(date)));
    }

    value = rec.value(h_due_date_);
    if (value.type() == QVariant::DateTime) {
        rec.setValue(h_due_date_, value.toDateTime().toTime_t());
    }

    return QSqlTableModel::updateRowInTable(row, rec);
}

QSqlRecord ActionsModel::getRecord()
{
    Q_ASSERT(intent_ > 0);
    Q_ASSERT(contact_ > 0);

    // Sequence must be above any sequence used for this intent so we get at the end
    QSqlQuery query(QStringLiteral("select max(sequence) from action where intent = %1").arg(intent_));
    query.next();
    const auto seq = query.value(0).toInt() + 1;

    auto today = QDateTime::currentDateTime();
    auto rec = record();
    const auto now = static_cast<uint>(time(nullptr));
    rec.setValue(h_created_date_, static_cast<uint>(now));
    rec.setValue(h_intent_, intent_);
    rec.setValue(h_contact_, contact_);
    rec.setValue(h_sequence_, seq);
    rec.setValue(h_start_date_, static_cast<uint>(ToTime(today.date())));
    rec.setValue(h_state_, 0);
    rec.setValue(h_type_, 0);

    qDebug() << "ActionsModel: start date is " << today.date();

    // Calculate number of days to get /n/ weekdays
    auto due = today;
    for(int due_days = 3; due_days > 0;) {
        due = due.addDays(1);

        // 1 = Monday to 7 = Sunday
        if (due.date().dayOfWeek() < 6) {
            --due_days; // loop stops when this is 0
        }
    }

    due.fromTime_t(ToTime(due.date()));
    qDebug() << "ActionsModel: due date is " << due;
    rec.setValue(h_due_date_, static_cast<uint>(due.toTime_t()));

    return rec;
}
