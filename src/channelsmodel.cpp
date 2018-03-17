#include <set>

#include <QSqlQuery>
#include <QSqlError>
#include <QRunnable>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlField>

#include "src/strategy.h"

#include "src/channel.h"
#include "src/channeldialog.h"
#include "src/channelsmodel.h"

using namespace std;

ChannelsModel::ChannelsModel(QSettings &settings, QObject *parent, QSqlDatabase db)
    : QSqlTableModel{parent, std::move(db)}
    , settings_{settings}
{
    setTable("channel");
    setEditStrategy(QSqlTableModel::OnFieldChange);

    h_id_ = fieldIndex("id");
    h_contact_ = fieldIndex("contact");
    h_type_ = fieldIndex("type");
    h_value_ = fieldIndex("value");
    h_verified_ = fieldIndex("verified");

    Q_ASSERT(h_contact_ > 0
            && h_type_ > 0
            && h_value_ > 0
            && h_verified_ > 0
    );

    setSort(h_value_, Qt::AscendingOrder);
    setFilter("id = -1"); // Filter everything away
}

void ChannelsModel::setContact(int id)
{
    setFilter(QStringLiteral("contact = %1").arg(id));
    select();
}

void ChannelsModel::removeChannels(const QModelIndexList &indexes)
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

void ChannelsModel::verifyChannels(const QModelIndexList &indexes, bool verified)
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);

    set<int> rows;
    for(const auto& ix : indexes) {
        rows.insert(ix.row());
    }

    for(const int row : rows) {
        const auto ix = index(row, h_verified_, {});
        setData(ix, verified);
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new contact (submitAll): "
                   << lastError().text();
    }
}

void ChannelsModel::addChannel(const QSqlRecord &rec)
{
    Strategy strategy(*this, QSqlTableModel::OnManualSubmit);


//    for(int i = 0; i < rec.count(); ++i) {
//        qDebug() << "# " << i << " " << rec.fieldName(i)
//                 << " " << rec.value(i).typeName()
//                 << " : " << (rec.isNull(i) ? QStringLiteral("NULL") : rec.value(i).toString());
//    }

    if (!insertRecord(-1, rec)) {
        qWarning() << "Failed to add new channel (insertRecord): "
                   << lastError().text();
        return;
    }

    if (!submitAll()) {
        qWarning() << "Failed to add new channel (submitAll): "
                   << lastError().text();
        return;
    }

    qDebug() << "Created new channel";
}


QVariant ChannelsModel::data(const QModelIndex &ix, int role) const
{
    static const QIcon check_icon{":/res/icons/check.svg"};

    if (role == Qt::DecorationRole && ix.column() == h_value_) {
        auto nix = index(ix.row(), h_type_, {});
        int status = QSqlTableModel::data(nix, Qt::DisplayRole).toInt();
        return GetChannelStatusIcon(status);
    }

    if (ix.column() == h_verified_) {
        if (role == Qt::DecorationRole) {
            auto nix = index(ix.row(), h_verified_, {});
            if (QSqlTableModel::data(nix, Qt::DisplayRole).toBool()) {
                return check_icon;
            }
        }

        if (role == Qt::DisplayRole) {
            return {};
        }
    }


    return QSqlTableModel::data(ix, role);
}

QVariant ChannelsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == h_value_) {
            return "Channel";
        } else {
            auto name = QSqlTableModel::headerData(section, orientation, role).toString();
            name[0] = name[0].toUpper();
            return name;
        }
    }

    return QSqlTableModel::headerData(section, orientation, role);
}
