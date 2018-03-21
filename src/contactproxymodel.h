#ifndef CONTACTPROXYMODEL_H
#define CONTACTPROXYMODEL_H

#include "contact.h"
#include "contactsmodel.h"

#include <QSortFilterProxyModel>
#include <QModelIndex>

#include "document.h"
#include "documentsmodel.h"

class ContactProxyModel : public QSortFilterProxyModel
{
public:
    ContactProxyModel(ContactsModel *docModel, QObject *parent = Q_NULLPTR);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    ContactsModel *model_ = {};

};

#endif // CONTACTPROXYMODEL_H
