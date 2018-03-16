#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include <QSettings>
#include <QSqlTableModel>
#include <QImage>
#include <QMetaType>
#include <QSqlDatabase>

#include "database.h"

class ContactsModel : public QSqlTableModel
{
    Q_OBJECT

public:
    ContactsModel(QSettings& settings, QObject *parent, QSqlDatabase db);
    ~ContactsModel() = default;

    int getNameCol() const noexcept { return h_name_; }

    QModelIndex createContact();

public:
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

public slots:


private:
    QSettings& settings_;

    int h_id_ = {};
    int h_name_ = {};
    int h_first_name_ = {};
    int h_last_name_ = {};
    int h_middle_name = {};
    int h_gender = {};
    int h_type_ = {};
    int h_status_ = {};
    int h_notes_ = {};
    int h_stars_ = {};
    int h_favorite_ = {};

    mutable bool internal_edit_ = false;
};

#endif // CONTACTSMODEL_H
