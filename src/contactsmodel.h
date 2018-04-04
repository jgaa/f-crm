#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include <QSettings>
#include <QSqlTableModel>
#include <QImage>
#include <QMetaType>
#include <QSqlDatabase>

#include "database.h"
#include "contact.h"

// Create read-only properties like 'name_col' for the database columns
#define DEF_COLUMN(name) Q_PROPERTY(int name ## _col MEMBER h_ ## name ## _)

class ContactsModel : public QSqlTableModel
{
    Q_OBJECT

public:
    ContactsModel(QSettings& settings, QObject *parent, QSqlDatabase db);
    ~ContactsModel() = default;

    DEF_COLUMN(id)
    DEF_COLUMN(contact)
    DEF_COLUMN(created_date)
    DEF_COLUMN(last_activity_date)
    DEF_COLUMN(name)
    DEF_COLUMN(gender)
    DEF_COLUMN(type)
    DEF_COLUMN(status)
    DEF_COLUMN(notes)
    DEF_COLUMN(stars)
    DEF_COLUMN(favorite)
    DEF_COLUMN(address1)
    DEF_COLUMN(address2)
    DEF_COLUMN(city)
    DEF_COLUMN(postcode)
    DEF_COLUMN(country)

    QModelIndex createContact(const ContactType type);

public:
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int getContactId(const QModelIndex& ix) const;

public slots:
    void setNameFilter(const QString& filter);

    // If used, this instance is for contacts belonging to a company (parent contact)
    void setParent(int contact);
    void removeContacts(const QModelIndexList& indexes);
    void addPerson(const QSqlRecord& rec);
    void toggleFavoriteStatus(const int row);
    void setStars(const int row, const int stars);

private:
    bool insertContact(QSqlRecord& rec);
    static const QIcon& getFavoriteIcon(const bool enable);
    static const QIcon& getStars(const int stars);

    QSettings& settings_;

    int h_id_ = {};
    int h_contact_ = {};
    int h_created_date_ = {};
    int h_last_activity_date_ = {};
    int h_name_ = {};
    int h_gender_ = {};
    int h_type_ = {};
    int h_status_ = {};
    int h_notes_ = {};
    int h_stars_ = {};
    int h_favorite_ = {};
    int h_address1_ = {};
    int h_address2_ = {};
    int h_city_ = {};
    int h_postcode_ = {};
    int h_region_ = {};
    int h_state_ = {}; // State as in Alabama,
    int h_country_ = {};

    mutable bool internal_edit_ = false;
    int parent_ = {};
};

#undef DEF_COLUMN

#endif // CONTACTSMODEL_H
