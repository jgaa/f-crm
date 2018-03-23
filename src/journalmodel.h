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


class JournalModel : public QSqlTableModel
{
    Q_OBJECT
public:
    enum class Type {
        GENERAL,
        ADD_COMPANY,
        ADD_PERSON,
        UPDATED_CONTACT,
        UPDATED_PERSON,
        ADD_DOCUMENT,
        UPDATED_DOCUMENT,
        DELETED_DOCUMENT,
        ADD_INTENT,
        UPDATE_INTENT,
        DELETE_INTENT,
        ADD_ACTION,
        EDIT_ACTION,
        DELETE_ACTION,
        DELETED_SOMETHING,
    };

    JournalModel(QSettings& settings, QObject *parent, QSqlDatabase db);

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

    static JournalModel& instance() {
        Q_ASSERT(instance_);
        return *instance_;
    }

public slots:
    void addEntry(QSqlRecord& rec); // Will modify rec
    //void addContactLog(const int contact, const Type type, const QString& text);
    void addEntry(const Type type, const QString& text,
                const int contact, const int person = 0, const int intent = 0,
                const int activity = 0, const int document = 0);

private:
    const QIcon& getLogIcon(int type) const;

    QSettings& settings_;
    static JournalModel *instance_;

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
