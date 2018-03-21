#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QSettings>
#include <QSqlTableModel>
#include <QImage>
#include <QMetaType>
#include <QSqlDatabase>

#include "database.h"

// Create read-only properties like 'name_col' for the database columns
#define DEF_COLUMN(name) Q_PROPERTY(int name ## _col MEMBER h_ ## name ## _)


class LogModel : public QSqlTableModel
{
    Q_OBJECT
public:
    enum class Type {
        GENERAL,
        ADD_COMPANY,
        ADD_PERSON,
        DELETED_SOMETHING,
    };

    LogModel(QSettings& settings, QObject *parent, QSqlDatabase db);

    DEF_COLUMN(id)
    DEF_COLUMN(type)
    DEF_COLUMN(date)
    DEF_COLUMN(contact)
    DEF_COLUMN(person)
    DEF_COLUMN(intent)
    DEF_COLUMN(channel)
    DEF_COLUMN(activity)
    DEF_COLUMN(document)
    DEF_COLUMN(text)

    void setContact(int id);

    static LogModel& instance() {
        Q_ASSERT(instance_);
        return *instance_;
    }

public slots:
    void addLog(QSqlRecord& rec); // Will modify rec
    void addContactLog(const int contact, const Type type, const QString& text);

private:
    const QIcon& getLogIcon(int type) const;

    QSettings& settings_;
    static LogModel *instance_;

    int h_id_ = {};
    int h_type_ = {};
    int h_date_ = {};
    int h_contact_ = {};
    int h_person_ = {};
    int h_intent_ = {};
    int h_channel_ = {};
    int h_activity_ = {};
    int h_document_ = {};
    int h_text_ = {};

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};


#undef DEF_COLUMN

#endif // LOGMODEL_H
