#include "src/logmodel.h"

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

LogModel *LogModel::instance_;

LogModel::LogModel(QSettings &settings, QObject *parent, QSqlDatabase db)
    : QSqlTableModel{parent, std::move(db)}
    , settings_{settings}
{
    Q_ASSERT(!instance_);

    instance_ = this;

    setTable("log");
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

void LogModel::setContact(int id)
{
    setFilter(QStringLiteral("contact = %1").arg(id));
    select();
}


void LogModel::addLog(QSqlRecord& rec)
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);

    const auto now = static_cast<uint>(time(nullptr));
    rec.setValue(h_date_, now);

    Q_ASSERT(!rec.value(h_type_).isNull());
    Q_ASSERT(!rec.value(h_text_).isNull());

//    for(int i = 0; i < rec.count(); ++i) {
//        qDebug() << "# " << i << " " << rec.fieldName(i)
//                 << " " << rec.value(i).typeName()
//                 << " : " << (rec.isNull(i) ? QStringLiteral("NULL") : rec.value(i).toString());
//    }

    if (!insertRecord(-1, rec)) {
        qWarning() << "Failed to add new intent (insertRecord): "
                   << lastError().text();
        return;
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new intent (submitAll): "
                   << lastError().text();
        return;
    }

    qDebug() << "Created new log entry";
}

void LogModel::addContactLog(const int contact,
                             const LogModel::Type type,
                             const QString &text)
{
    auto rec = record();
    rec.setValue(h_contact_, contact);
    rec.setValue(h_type_, static_cast<int>(type));
    rec.setValue(h_text_, text);

    addLog(rec);
}

const QIcon &LogModel::getLogIcon(int type) const
{
    static const array<QIcon, 4> icons {{
        QIcon(":/res/icons/log_general.svg"),
        QIcon(":/res/icons/addcompany.svg"),
        QIcon(":/res/icons/addperson.svg"),
        QIcon(":/res/icons/delete.svg"),
    }};

    return icons.at(static_cast<size_t>(type));
}

QVariant LogModel::data(const QModelIndex &ix, int role) const
{
    if (ix.isValid()) {
        if (role == Qt::DisplayRole) {
            if (ix.column() == h_date_) {
                const auto when = QDateTime::fromSecsSinceEpoch(QSqlTableModel::data(ix, Qt::DisplayRole).toLongLong());
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

QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        auto name = QSqlTableModel::headerData(section, orientation, role).toString();
        name[0] = name[0].toUpper();
        return name;
    }
    return QSqlTableModel::headerData(section, orientation, role);
}
