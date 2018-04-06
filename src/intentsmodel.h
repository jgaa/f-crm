#ifndef INTENTSMODEL_H
#define INTENTSMODEL_H

#include <QSettings>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QImage>
#include <QMetaType>
#include <QSqlDatabase>

#include "database.h"

// Create read-only properties like 'name_col' for the database columns
#define DEF_COLUMN(name) Q_PROPERTY(int name ## _col MEMBER h_ ## name ## _)


class IntentsModel : public QSqlTableModel
{
    Q_OBJECT
public:
    IntentsModel(QSettings& settings, QObject *parent, QSqlDatabase db);

    DEF_COLUMN(id)
    DEF_COLUMN(contact)
    DEF_COLUMN(type)
    DEF_COLUMN(state)
    DEF_COLUMN(abstract)
    DEF_COLUMN(notes)
    DEF_COLUMN(created_date)

    void setContact(int id);
    int getIntentId(const QModelIndex& ix);

public slots:
    void removeIntents(const QModelIndexList& indexes);
    void addIntent(QSqlRecord rec);
    void updateState();

private:
    QSettings& settings_;

    int h_id_ = {};
    int h_contact_ = {};
    int h_type_ = {};
    int h_state_ = {};
    int h_abstract_ = {};
    int h_notes_ = {};
    int h_created_date_ = {};

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};


#undef DEF_COLUMN

#endif // INTENTSMODEL_H
