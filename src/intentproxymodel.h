#ifndef INTENTPROXYMODEL_H
#define INTENTPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QModelIndex>

#include "intent.h"
#include "intentsmodel.h"

class IntentProxyModel : public QSortFilterProxyModel
{
public:
    IntentProxyModel(IntentsModel *docModel, QObject *parent = Q_NULLPTR);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    IntentsModel *model_ = {};

};

#endif // INTENTPROXYMODEL_H
