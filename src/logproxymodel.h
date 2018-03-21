#ifndef LOGPROXYMODEL_H
#define LOGPROXYMODEL_H


#include <QSortFilterProxyModel>
#include <QModelIndex>

#include "logmodel.h"

class LogProxyModel : public QSortFilterProxyModel
{
public:
    LogProxyModel(LogModel *docModel, QObject *parent = Q_NULLPTR);

    // QAbstractItemModel interface
public:
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    LogModel *model_ = {};

};

#endif // LOGPROXYMODEL_H
