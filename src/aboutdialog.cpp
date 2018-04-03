#include "src/aboutdialog.h"
#include "ui_aboutdialog.h"
#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->version->setText(QString("Version ") + F_CRM_VERSION);
    ui->icon->setPixmap(QIcon(":res/icons/f-crm.svg").pixmap({ui->icon->width(), ui->icon->height()}));

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(close()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
