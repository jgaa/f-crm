#ifndef LOGPROXYMODEL_H
#define LOGPROXYMODEL_H


#include <QSortFilterProxyModel>
#include <QModelIndex>

#include "journalmodel.h"

class JournalProxyModel : public QSortFilterProxyModel
{
public:
    JournalProxyModel(JournalModel *docModel, QObject *parent = Q_NULLPTR);

    // QAbstractItemModel interface
public:
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    JournalModel *model_ = {};

};

#endif // LOGPROXYMODEL_H
