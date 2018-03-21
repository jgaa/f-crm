#ifndef DOCUMENTPROXYMODEL_H
#define DOCUMENTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QModelIndex>

#include "document.h"
#include "documentsmodel.h"

class DocumentProxyModel : public QSortFilterProxyModel
{
public:
    DocumentProxyModel(DocumentsModel *docModel, QObject *parent = Q_NULLPTR);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    DocumentsModel *model_ = {};

};

#endif // DOCUMENTPROXYMODEL_H
