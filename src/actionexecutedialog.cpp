#include "src/actionexecutedialog.h"
#include "ui_actionexecutedialog.h"

ActionExecuteDialog::ActionExecuteDialog(const QList<QString>& data,
                                         const int type,
                                         QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActionExecuteDialog)
{
    ui->setupUi(this);

    auto icon = GetChannelStatusIcon(type);

    for(const auto& v : data ) {
        ui->channels->addItem(new QListWidgetItem(icon, v, ui->channels));
    }

    ui->channels->setCurrentRow(0);
}


ActionExecuteDialog::~ActionExecuteDialog()
{
    delete ui;
}


void ActionExecuteDialog::accept()
{
    value = ui->channels->currentItem()->text();
    QDialog::accept();
}
