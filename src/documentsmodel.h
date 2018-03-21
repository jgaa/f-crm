#ifndef DOCUMENTSMODEL_H
#define DOCUMENTSMODEL_H


#include <QSettings>
#include <QSqlTableModel>
#include <QImage>
#include <QMetaType>
#include <QSqlDatabase>

#include "database.h"
#include "document.h"

// Create read-only properties like 'name_col' for the database columns
#define DEF_COLUMN(name) Q_PROPERTY(int name ## _col MEMBER h_ ## name ## _)


class DocumentsModel : public QSqlTableModel
{
    Q_OBJECT
public:
    DocumentsModel(QSettings& settings, QObject *parent, QSqlDatabase db);

    DEF_COLUMN(id)
    DEF_COLUMN(contact)
    DEF_COLUMN(person)
    DEF_COLUMN(intent)
    DEF_COLUMN(activity)
    DEF_COLUMN(type)
    DEF_COLUMN(cls)
    DEF_COLUMN(direction)
    DEF_COLUMN(entity)
    DEF_COLUMN(name)
    DEF_COLUMN(notes)
    DEF_COLUMN(added_date)
    DEF_COLUMN(file_date)
    DEF_COLUMN(location)
    DEF_COLUMN(content)

    void setContact(int id);

    // Get a record with default values
    QSqlRecord getRecord(int contact, Document::Type type,
                         Document::Class cls, Document::Direction direction,
                         Document::Entity entity = Document::Entity::CONTACT,
                         int person = 0, int intent = 0, int action = 0);

public slots:
    void removeDocuments(const QModelIndexList& indexes);
    void addDocument(const QSqlRecord& rec);
    void updateDocument(const int row, const QSqlRecord& rec);

private:
    void fix(QSqlRecord& rec);


    QSettings& settings_;

    int h_id_ = {};
    int h_contact_ = {};
    int h_person_ = {};
    int h_intent_ = {};
    int h_activity_ = {};
    int h_type_ = {};
    int h_cls_ = {};
    int h_direction_ = {};
    int h_entity_ = {};
    int h_name_ = {};
    int h_notes_ = {};
    int h_added_date_ = {};
    int h_file_date_ = {};
    int h_location_ = {};
    int h_content_ = {};

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // QSqlTableModel interface
protected:
    bool updateRowInTable(int row, const QSqlRecord &values) override;
};


#endif // DOCUMENTSMODEL_H
