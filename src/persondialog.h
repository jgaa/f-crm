#ifndef PERSONDIALOG_H
#define PERSONDIALOG_H

#include <QSqlRecord>
#include <QDialog>
#include <QDataWidgetMapper>

#include "src/contactsmodel.h"


namespace Ui {
class PersonDialog;
}

class PersonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PersonDialog(QWidget *parent = 0);
    ~PersonDialog();

    void setRecord(const QSqlRecord& rec);
    void setModel(ContactsModel *model, QModelIndex& ix);

signals:
    void addPerson(const QSqlRecord& rec);

private:
    Ui::PersonDialog *ui;
    QSqlRecord rec_;
    QDataWidgetMapper *mapper_ = {};

    // QDialog interface
public slots:
    void accept() override;
    void reject() override;
};

#endif // PERSONDIALOG_H
