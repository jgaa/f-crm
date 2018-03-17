#include "src/persondialog.h"
#include "ui_persondialog.h"

PersonDialog::PersonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PersonDialog)
{
    ui->setupUi(this);
}

PersonDialog::~PersonDialog()
{
    delete ui;
}
