#include "src/upcomingmodel.h"

#include <QDateTime>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

#include "action.h"
#include "contact.h"

UpcomingModel::UpcomingModel(QSettings &settings, QObject *parent, Mode mode)
    : QSqlQueryModel{parent}
    , mode_{mode}
    , settings_{settings}
{
    select();
}

void UpcomingModel::setContact(int contact)
{
    contact_ = contact;
    select();
}

void UpcomingModel::select()
{
    //beginResetModel();
    setQuery(createQuery());
    //endResetModel();
}

QSqlQuery UpcomingModel::createQuery() const
{
    QString where_statement;

    switch(mode_) {

    case Mode::CONTACT_UPCOMING:
        where_statement = QStringLiteral(
                    "a.contact = %1 AND a.state < 4 AND a.start_date < (strftime('%s', date('now')))")
                .arg(contact_);
        break;
    case Mode::TODAY:
        where_statement = QStringLiteral(
                    "a.state == 1 AND a.start_date < (strftime('%s', date('now')))");
        break;
    case Mode::UPCOMING:
        where_statement = QStringLiteral(
                    "a.state < 4 AND a.start_date < (strftime('%s', date('now')))");
        break;
    }

    const auto sql_statement = QStringLiteral(
            "SELECT a.id, a.state, a.start_date, a.contact, c.name,  c.status, a.intent, i.abstract, a.person, p.name, a.name, a.due_date, a.desired_outcome "
                 "FROM action as a "
                 "LEFT JOIN contact as c on c.id = a.contact "
                 "LEFT JOIN intent as i on i.id = a.intent "
                 "LEFT JOIN contact as p on p.id = a.person "
                 "WHERE %1 "
                 "ORDER BY a.start_date ASC "
                ).arg(where_statement);

    QSqlQuery query;
    if (!query.exec(sql_statement)) {
        qWarning() << "Failed to query for the actions: " << query.lastError()
                   << " Query: " << sql_statement;
    }

    return query;
}

QVariant UpcomingModel::data(const QModelIndex &ix, int role) const
{
    if (ix.isValid()) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {

            if (ix.column() == H_START_DATE
                    || ix.column() == H_DUE_DATE) {
                const auto when = QDateTime::fromTime_t(
                            QSqlQueryModel::data(ix, Qt::DisplayRole).toUInt());
                return when.date();
            }

        } else if (role == Qt::DecorationRole) {
            if (ix.column() == H_START_DATE) {
                const auto cix = index(ix.row(), H_STATE, {});
                return data(cix, role);
            }

            if (ix.column() == H_CONTACT_NAME) {
                const auto cix = index(ix.row(), H_CONTACT_STATUS, {});
                return data(cix, role);
            }

            if (ix.column() == H_STATE) {
                return GetActionStateIcon(std::max(0, QSqlQueryModel::data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == H_STATE) {
                return GetActionStateIcon(std::max(0, QSqlQueryModel::data(ix, Qt::DisplayRole).toInt()));
            }

            if (ix.column() == H_CONTACT_STATUS) {
                return GetContactStatusIcon(std::max(0, QSqlQueryModel::data(ix, Qt::DisplayRole).toInt()));
            }

        } else if (role == Qt::ToolTipRole) {
            const auto cix = index(ix.row(), H_DESIRED_OUTCOME, {});
            return QSqlQueryModel::data(cix, Qt::DisplayRole).toString();
        }
    }

    return QSqlQueryModel::data(ix, role);
}
