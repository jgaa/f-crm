#ifndef ACTIONPROXYMODEL_H
#define ACTIONPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QModelIndex>

#include "action.h"
#include "actionsmodel.h"

class ActionProxyModel : public QSortFilterProxyModel
{
public:
    ActionProxyModel(ActionsModel *docModel, QObject *parent = Q_NULLPTR);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    ActionsModel *model_ = {};

};


#endif // ACTIONPROXYMODEL_H
