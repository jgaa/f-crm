#include "src/journalmodel.h"

#include <set>
#include <QSqlQuery>
#include <QSqlError>
#include <QRunnable>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlField>
#include <QDateTime>

#include "src/strategy.h"
#include "src/intent.h"

using namespace std;

JournalModel *JournalModel::instance_;

JournalModel::JournalModel(QSettings &settings, QObject *parent, QSqlDatabase db)
    : QSqlTableModel{parent, std::move(db)}
    , settings_{settings}
{
    Q_ASSERT(!instance_);

    instance_ = this;

    setTable("journal");
    setEditStrategy(QSqlTableModel::OnFieldChange);

    h_id_ = fieldIndex("id");
    h_type_ = fieldIndex("type");
    h_date_ = fieldIndex("date");
    h_contact_ = fieldIndex("contact");
    h_person_ = fieldIndex("person");
    h_intent_ = fieldIndex("intent");
    h_channel_ = fieldIndex("channel");
    h_activity_ = fieldIndex("activity");
    h_document_ = fieldIndex("document");
    h_text_ = fieldIndex("text");

    Q_ASSERT(h_id_ >= 0
            && h_type_ > 0
            && h_date_ > 0
            && h_contact_ > 0
            && h_person_ > 0
            && h_intent_ > 0
            && h_channel_ > 0
            && h_activity_ > 0
            && h_document_ > 0
            && h_text_ > 0
         );

    setSort(h_date_, Qt::AscendingOrder);
    setFilter("id = -1"); // Filter everything away
}

void JournalModel::setContact(int id)
{
    setFilter(QStringLiteral("contact = %1").arg(id));
    select();
}


void JournalModel::addEntry(QSqlRecord& rec)
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);

    const auto now = static_cast<uint>(time(nullptr));
    rec.setValue(h_date_, now);

    Q_ASSERT(!rec.value(h_type_).isNull());
    Q_ASSERT(!rec.value(h_text_).isNull());

    qDebug() << "Adding Journal entry. Fields: ";
    for(int i = 0; i < rec.count(); ++i) {
        qDebug() << "  # " << i << " " << rec.fieldName(i)
        << " " << rec.value(i).typeName()
        << " : " << (rec.isNull(i) ? QStringLiteral("NULL") : rec.value(i).toString());
    }

    if (!insertRecord(-1, rec)) {
        qWarning() << "Failed to add new journal (insertRecord): "
                   << lastError().text();
        return;
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new journal (submitAll): "
                   << lastError().text();
        return;
    }

    qDebug() << "Created new log entry";
}

void JournalModel::addEntry(const JournalModel::Type type, const QString &text,
                      const int contact, const int person,
                      const int intent, const int activity,
                      const int document)
{
    auto rec = record();
    rec.setValue(h_type_, static_cast<int>(type));
    rec.setValue(h_text_, text);

    if (contact > 0)
        rec.setValue(h_contact_, contact);

    if (person > 0)
        rec.setValue(h_person_, person);

    if (intent > 0)
        rec.setValue(h_intent_, intent);

    if (activity > 0)
        rec.setValue(h_activity_, activity);

    if (document > 0)
        rec.setValue(h_document_, document);

    addEntry(rec);
}

const QIcon &JournalModel::getLogIcon(int type) const
{
    static const array<QIcon, 15> icons {{
        QIcon(":/res/icons/log_general.svg"),
        QIcon(":/res/icons/addcompany.svg"),
        QIcon(":/res/icons/addperson.svg"),
        QIcon(":/res/icons/updated_company.svg"),
        QIcon(":/res/icons/updated_person.svg"),
        QIcon(":/res/icons/add_document.svg"),
        QIcon(":/res/icons/edit_document.svg"),
        QIcon(":/res/icons/delete_document.svg"),
        QIcon(":/res/icons/add_intent.svg"),
        QIcon(":/res/icons/edit_intent.svg"),
        QIcon(":/res/icons/delete_intent.svg"),
        QIcon(":/res/icons/add_action.svg"),
        QIcon(":/res/icons/edit_action.svg"),
        QIcon(":/res/icons/delete_action.svg"),
        QIcon(":/res/icons/delete.svg"),
    }};

    return icons.at(static_cast<size_t>(type));
}

QVariant JournalModel::data(const QModelIndex &ix, int role) const
{
    if (ix.isValid()) {
        if (role == Qt::DisplayRole) {
            if (ix.column() == h_date_) {
                const auto when = QDateTime::fromTime_t(QSqlTableModel::data(ix, Qt::DisplayRole).toLongLong());
                return when.toString("yyyy-MM-dd hh:mm");
            }
        } else if (role == Qt::DecorationRole) {
            if (ix.column() == h_type_) {
                return getLogIcon(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_date_) {
                return getLogIcon(std::max(0, QSqlTableModel::data(index(ix.row(), h_type_, {}), Qt::DisplayRole).toInt()));
            }
        }
    }

    return QSqlTableModel::data(ix, role);
}

QVariant JournalModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        auto name = QSqlTableModel::headerData(section, orientation, role).toString();
        name[0] = name[0].toUpper();
        return name;
    }
    return QSqlTableModel::headerData(section, orientation, role);
}
