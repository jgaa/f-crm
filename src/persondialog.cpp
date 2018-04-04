#include <tuple>

#include "contact.h"
#include "src/persondialog.h"
#include "ui_persondialog.h"

PersonDialog::PersonDialog(ContactsModel& model, bool isPerson, int row, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PersonDialog),
    is_person_{isPerson},
    row_{row},
    model_{model}
{
    ui->setupUi(this);

    this->setWindowTitle(QStringLiteral("Edit %1")
                         .arg(isPerson ? "Person" : "Contact"));

    mapper_.setModel(&model_);

    for(auto e : GetContactStatusEnums()) {
         ui->status->addItem(GetContactStatusIcon(e),
                             GetContactStatusName(e),
                             static_cast<int>(e));
    }

    if (is_person_) {
        for(auto e : GetContactGenderEnums()) {
             ui->gender->addItem(GetContactGenderIcon(e),
                                 GetContactGenderName(e),
                                 static_cast<int>(e));
        }

        mapper_.addMapping(ui->gender, model_.fieldIndex("gender"), "currentData");
        ui->status->setHidden(true);
        ui->statusLabel->setHidden(true);
    } else {
        mapper_.addMapping(ui->status, model_.fieldIndex("status"), "currentData");
        ui->gender->setHidden(true);
        ui->genderLabel->setHidden(true);
    }

    for(int i = 0; i <= 5; i++) {
        ui->stars->addItem(QIcon(QStringLiteral(":/res/icons/%1star").arg(i)), "", i);
    }

    ui->stars->setIconSize({80, 16});

    mapper_.addMapping(ui->stars, model_.fieldIndex("stars"), "currentData");
    mapper_.addMapping(ui->favorite, model_.fieldIndex("favourite"));
    mapper_.addMapping(ui->name, model_.fieldIndex("name"));
    mapper_.addMapping(ui->address1, model_.fieldIndex("address1"));
    mapper_.addMapping(ui->address2, model_.fieldIndex("address2"));
    mapper_.addMapping(ui->city, model_.fieldIndex("city"));
    mapper_.addMapping(ui->postcode, model_.fieldIndex("postcode"));
    mapper_.addMapping(ui->region, model_.fieldIndex("region"));
    mapper_.addMapping(ui->state, model_.fieldIndex("state"));
    mapper_.addMapping(ui->country, model_.fieldIndex("country"));
    mapper_.addMapping(ui->notes, model_.fieldIndex("notes"));

    mapper_.setCurrentIndex(row);

    {
        const auto dix = model_.index(row, model_.fieldIndex("stars"), {});
        int gender_val = model_.data(dix, Qt::EditRole).toInt();
        ui->stars->setCurrentIndex(ui->stars->findData(gender_val));
    }

    if (is_person_) {
        const auto dix = model_.index(row, model_.fieldIndex("gender"), {});
        int gender_val = model_.data(dix, Qt::EditRole).toInt();
        ui->gender->setCurrentIndex(ui->gender->findData(gender_val));
    } else {
        const auto dix = model_.index(row, model_.fieldIndex("status"), {});
        int status_val = model_.data(dix, Qt::EditRole).toInt();
        ui->status->setCurrentIndex(ui->status->findData(status_val));
    }
}

PersonDialog::~PersonDialog()
{
    delete ui;
}

void PersonDialog::accept()
{
    mapper_.submit();

    QDialog::accept();
}


