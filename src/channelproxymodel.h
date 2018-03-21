#ifndef CHANNELPROXYMODEL_H
#define CHANNELPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QModelIndex>

#include "channel.h"
#include "channelsmodel.h"

class ChannelProxyModel : public QSortFilterProxyModel
{
public:
    ChannelProxyModel(ChannelsModel *docModel, QObject *parent = Q_NULLPTR);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    ChannelsModel *model_ = {};

};

#endif // CHANNELPROXYMODEL_H
