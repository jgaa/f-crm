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
    explicit PersonDialog(const QSqlRecord& rec, bool isPerson, int row, QWidget *parent = 0);
    ~PersonDialog();

signals:
    void addPerson(const QSqlRecord& rec);
    void updatePerson(const int row, const QSqlRecord& rec);
    void setFilter(QString value);

private:
    Ui::PersonDialog *ui;
    QSqlRecord rec_;
    const bool is_person_;
    const int row_;

    // QDialog interface
public slots:
    void accept() override;
};

#endif // PERSONDIALOG_H
