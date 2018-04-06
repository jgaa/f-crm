#ifndef CHANNELSMODEL_H
#define CHANNELSMODEL_H

#include <QSettings>
#include <QSqlTableModel>
#include <QImage>
#include <QMetaType>
#include <QSqlDatabase>

#include "database.h"

// Create read-only properties like 'name_col' for the database columns
#define DEF_COLUMN(name) Q_PROPERTY(int name ## _col MEMBER h_ ## name ## _)


class ChannelsModel : public QSqlTableModel
{
    Q_OBJECT
public:
    ChannelsModel(QSettings& settings, QObject *parent, QSqlDatabase db);

    DEF_COLUMN(id)
    DEF_COLUMN(contact)
    DEF_COLUMN(type)
    DEF_COLUMN(value)
    DEF_COLUMN(verified)
    DEF_COLUMN(name)

    void setContact(int id);

public slots:
    void removeChannels(const QModelIndexList& indexes);
    void verifyChannels(const QModelIndexList& indexes, bool verified = true);
    void addChannel(const QSqlRecord& rec);

private:
    QSettings& settings_;

    int h_id_ = {};
    int h_contact_ = {};
    int h_type_ = {};
    int h_value_ = {};
    int h_verified_ = {};
    int h_name_ = {};

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};


#undef DEF_COLUMN

#endif // CHANNELSMODEL_H
