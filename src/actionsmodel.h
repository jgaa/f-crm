#ifndef ACTIONSMODEL_H
#define ACTIONSMODEL_H


#include <QSettings>
#include <QSqlTableModel>
#include <QImage>
#include <QMetaType>
#include <QSqlDatabase>

#include "database.h"

// Create read-only properties like 'name_col' for the database columns
#define DEF_COLUMN(name) Q_PROPERTY(int name ## _col MEMBER h_ ## name ## _)


class ActionsModel : public QSqlTableModel
{
    Q_OBJECT
public:
    ActionsModel(QSettings& settings, QObject *parent, QSqlDatabase db);

    DEF_COLUMN(id)
    DEF_COLUMN(sequence)
    DEF_COLUMN(intent)
    DEF_COLUMN(contact)
    DEF_COLUMN(person)
    DEF_COLUMN(state)
    DEF_COLUMN(type)
    DEF_COLUMN(channel_type)
    DEF_COLUMN(name)
    DEF_COLUMN(created_date)
    DEF_COLUMN(start_date)
    DEF_COLUMN(due_date)
    DEF_COLUMN(desired_outcome)
    DEF_COLUMN(notes)

    void setContact(int id);
    void setIntent(int id);
    int contact() const { return contact_; }

    // Get a record with default values
    QSqlRecord getRecord();

public slots:
    void removeActions(const QModelIndexList& indexes);
    void addAction(const QSqlRecord& rec);
    void setCompleted(const QModelIndex& ix);
    void moveUp(const QModelIndex& ix);
    void moveDown(const QModelIndex& ix);
    void openNextActions();
    void updateState();

private:
    void doMove(const QModelIndex &ix, const int offset);

    QSettings& settings_;

    int h_id_ = {};
    int h_sequence_ = {};
    int h_contact_ = {};
    int h_intent_ = {};
    int h_person_ = {};
    int h_state_ = {};
    int h_type_ = {};
    int h_channel_type_ = {};
    int h_name_ = {};
    int h_created_date_ = {};
    int h_start_date_ = {};
    int h_due_date_ = {};
    int h_desired_outcome_ = {};
    int h_notes_ = {};

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int contact_ = {};
    int intent_ = {};

    // QSqlTableModel interface
protected:
    bool updateRowInTable(int row, const QSqlRecord &values) override;
};


#undef DEF_COLUMN

#endif // ACTIONSMODEL_H
