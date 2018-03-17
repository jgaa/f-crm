#ifndef PERSONDIALOG_H
#define PERSONDIALOG_H

#include <QDialog>

namespace Ui {
class PersonDialog;
}

class PersonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PersonDialog(QWidget *parent = 0);
    ~PersonDialog();

private:
    Ui::PersonDialog *ui;
};

#endif // PERSONDIALOG_H
