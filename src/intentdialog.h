#ifndef INTENTDIALOG_H
#define INTENTDIALOG_H

#include <QSqlRecord>
#include <QDialog>
#include <QDataWidgetMapper>

#include "src/intentsmodel.h"

namespace Ui {
class IntentDialog;
}

class IntentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IntentDialog(QWidget *parent = 0);
    ~IntentDialog();

    void setRecord(const QSqlRecord& rec);
    void setModel(IntentsModel *model, QModelIndex& ix);

signals:
    void addIntent(const QSqlRecord& rec);

private:
    Ui::IntentDialog *ui;
    QSqlRecord rec_;
    QDataWidgetMapper *mapper_ = {};


    // QDialog interface
public slots:
    void accept() override;
    void reject() override;
};

#endif // INTENTDIALOG_H
