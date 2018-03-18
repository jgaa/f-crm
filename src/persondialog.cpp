#include <tuple>

#include "contact.h"
#include "src/persondialog.h"
#include "ui_persondialog.h"

PersonDialog::PersonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PersonDialog)
{
    ui->setupUi(this);

    for(auto e : GetContactStatusEnums()) {
         ui->status->addItem(GetContactStatusIcon(e),
                             GetContactStatusName(e),
                             static_cast<int>(e));
    }

    for(auto e : GetContactGenderEnums()) {
         ui->gender->addItem(GetContactGenderIcon(e),
                             GetContactGenderName(e),
                             static_cast<int>(e));
    }
}

PersonDialog::~PersonDialog()
{
    delete ui;
}

void PersonDialog::setRecord(const QSqlRecord &rec)
{
    rec_ = rec;
}

void PersonDialog::setModel(ContactsModel *model, QModelIndex &ix)
{
    Q_ASSERT(mapper_ == nullptr);
    Q_ASSERT(model);
    Q_ASSERT(ix.isValid());

    mapper_ = new QDataWidgetMapper(this);

    mapper_->setModel(model);
    mapper_->addMapping(ui->status, model->property("status_col").toInt(), "currentData");
    mapper_->addMapping(ui->gender, model->property("gender_col").toInt(), "currentData");

    mapper_->addMapping(ui->name, model->property("name_col").toInt());
    mapper_->addMapping(ui->address1, model->property("address1_col").toInt());
    mapper_->addMapping(ui->address2, model->property("address2_col").toInt());
    mapper_->addMapping(ui->city, model->property("city_col").toInt());
    mapper_->addMapping(ui->postcode, model->property("postcode_col").toInt());
    mapper_->addMapping(ui->country, model->property("country_col").toInt());
    mapper_->addMapping(ui->notes, model->property("notes_col").toInt());

    mapper_->setCurrentIndex(ix.row());

    // The mapping is not smart enough to initialize combo boxes
    {
        const auto dix = model->index(ix.row(), model->property("status_col").toInt(), {});
        int status_val = model->data(dix, Qt::EditRole).toInt();
        ui->status->setCurrentIndex(status_val);
    }

    {
        const auto dix = model->index(ix.row(), model->property("gender_col").toInt(), {});
        int gender_val = model->data(dix, Qt::EditRole).toInt();
        ui->gender->setCurrentIndex(gender_val);
    }
}


void PersonDialog::accept()
{
    if (!rec_.isEmpty()) {
        rec_.setValue("status", ui->status->currentData().toInt());
        rec_.setValue("gender", ui->gender->currentData().toInt());
        rec_.setValue("name", ui->name->text());
        rec_.setValue("address1", ui->address1->text());
        rec_.setValue("address2", ui->address2->text());
        rec_.setValue("city", ui->city->text());
        rec_.setValue("postcode", ui->postcode->text());
        rec_.setValue("country", ui->country->text());
        rec_.setValue("notes", ui->notes->toPlainText());

        emit addPerson(rec_);
    }

    if (mapper_) {
        mapper_->submit();
    }

    QDialog::accept();
}

void PersonDialog::reject()
{
    if (mapper_) {
        mapper_->revert();
    }

    QDialog::reject();
}
