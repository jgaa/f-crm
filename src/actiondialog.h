#ifndef ACTIONDIALOG_H
#define ACTIONDIALOG_H

#include <QSqlRecord>
#include <QDialog>
#include <QDataWidgetMapper>

#include "actionsmodel.h"

namespace Ui {
class ActionDialog;
}

class ActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ActionDialog(const int contact, QWidget *parent = 0);
    ~ActionDialog();

    void setRecord(const QSqlRecord& rec);
    void setModel(ActionsModel *model, QModelIndex& ix);

signals:
    void addAction(const QSqlRecord& rec);

private:
    void checkAccess();

    Ui::ActionDialog *ui;
    QSqlRecord rec_;
    QDataWidgetMapper *mapper_ = {};

    // QDialog interface
public slots:
    void accept() override;
    void reject() override;

private slots:
    void onCurrentIndexChanged(int index);

};

#endif // ACTIONDIALOG_H
