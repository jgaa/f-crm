#ifndef CHANNELDIALOG_H
#define CHANNELDIALOG_H

#include <QSqlRecord>
#include <QDialog>
#include <QDataWidgetMapper>

#include "src/channelsmodel.h"

namespace Ui {
class ChannelDialog;
}

class ChannelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChannelDialog(QWidget *parent = 0);
    ~ChannelDialog();

    void setRecord(const QSqlRecord& rec);
    void setModel(ChannelsModel *model, QModelIndex& ix);

signals:
    void addChannel(const QSqlRecord& rec);

private:
    Ui::ChannelDialog *ui;
    QSqlRecord rec_;
    QDataWidgetMapper *mapper_ = {};

    // QDialog interface
public slots:
    void accept() override;
    void reject() override;
};

#endif // CHANNELDIALOG_H
