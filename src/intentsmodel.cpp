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

    Q_ASSERT(h_contact_ > 0
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

void IntentsModel::removeIntents(const QModelIndexList &indexes)
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);

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

    qDebug() << "Created new intent";
}

QVariant IntentsModel::data(const QModelIndex &ix, int role) const
{
    if (ix.isValid()) {
        if (role == Qt::DisplayRole) {
             //Just pass the defaults
        } else if (role == Qt::DecorationRole) {
            if (ix.column() == h_abstract_) {
                const auto cix = index(ix.row(), h_type_, {});
                return GetIntentTypeIcon(std::max(0, QSqlTableModel::data(cix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_state_) {
                return GetIntentStateIcon(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == h_type_) {
                return GetIntentTypeIcon(std::max(0, QSqlTableModel::data(ix, Qt::DisplayRole).toInt()));
            }
        }
    }

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
