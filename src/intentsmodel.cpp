#include <set>
#include <QSqlQuery>
#include <QSqlError>
#include <QRunnable>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlField>

#include "src/intentsmodel.h"
#include "src/strategy.h"
#include "src/intent.h"
#include "src/logmodel.h"

using namespace std;

IntentsModel::IntentsModel(QSettings &settings, QObject *parent, QSqlDatabase db)
    : QSqlTableModel{parent, std::move(db)}
    , settings_{settings}
{
    setTable("intent");
    setEditStrategy(QSqlTableModel::OnFieldChange);

    h_id_ = fieldIndex("id");
    h_contact_ = fieldIndex("contact");
    h_type_ = fieldIndex("type");
    h_state_ = fieldIndex("state");
    h_abstract_ = fieldIndex("abstract");
    h_notes_ = fieldIndex("notes");

    Q_ASSERT(h_id_ >= 0
            && h_contact_ > 0
            && h_type_ > 0
            && h_state_ > 0
            && h_abstract_ > 0
            && h_notes_ > 0
    );

    setSort(h_id_, Qt::AscendingOrder);
    setFilter("id = -1"); // Filter everything away
}

void IntentsModel::setContact(int id)
{
    setFilter(QStringLiteral("contact = %1").arg(id));
    select();
}

int IntentsModel::getIntentId(const QModelIndex &ix)
{
    if (!ix.isValid()) {
        return 0;
    }

    return data(index(ix.row(), h_id_, {}), Qt::DisplayRole).toInt();
}

void IntentsModel::removeIntents(const QModelIndexList &indexes)
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

        LogModel::instance().addLog(LogModel::Type::DELETE_INTENT,
                                    QStringLiteral("Deleted intent: %1")
                                    .arg(rec.value("abstract").toString()),
                                    rec.value("contact").toInt(), 0,
                                    rec.value("id").toInt());
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new contact (submitAll): "
                   << lastError().text();
    }
}

void IntentsModel::addIntent(const QSqlRecord &rec)
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);


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

    LogModel::instance().addLog(LogModel::Type::ADD_INTENT,
                                QStringLiteral("Added intent: %1")
                                .arg(rec.value("abstract").toString()),
                                rec.value("contact").toInt(), 0,
                                query().lastInsertId().toInt());

    qDebug() << "Created new intent";
}

QVariant IntentsModel::data(const QModelIndex &ix, int role) const
{
    return QSqlTableModel::data(ix, role);
}

QVariant IntentsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        auto name = QSqlTableModel::headerData(section, orientation, role).toString();
        name[0] = name[0].toUpper();
        return name;
    }
    return QSqlTableModel::headerData(section, orientation, role);
}
