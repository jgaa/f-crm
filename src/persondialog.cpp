#include <tuple>

#include "contact.h"
#include "src/persondialog.h"
#include "ui_persondialog.h"

PersonDialog::PersonDialog(const QSqlRecord& rec, bool isPerson, int row, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PersonDialog),
    rec_{rec},
    is_person_{isPerson},
    row_{row}
{
    ui->setupUi(this);

    this->setWindowTitle(QStringLiteral("%1 %2")
                         .arg(row >= 0 ? "Edit" : "Add")
                         .arg(isPerson ? "Person" : "Contact"));

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
        ui->gender->setCurrentIndex(ui->gender->findData(rec.value("gender").toInt()));
    } else {
        ui->gender->setHidden(true);
        ui->genderLabel->setHidden(true);
    }

    for(int i = 0; i <= 5; i++) {
        ui->stars->addItem(QIcon(QStringLiteral(":/res/icons/%1star").arg(i)), "", i);
    }

    ui->stars->setIconSize({80, 16});
    ui->stars->setCurrentIndex(rec.value("stars").toInt());
    ui->favorite->setChecked(rec.value("favourite").toBool());
    ui->status->setCurrentIndex(ui->status->findData(rec.value("status").toInt()));
    ui->name->setText(rec.value("name").toString());
    ui->address1->setText(rec.value("address1").toString());
    ui->address2->setText(rec.value("address2").toString());
    ui->city->setText(rec.value("city").toString());
    ui->postcode->setText(rec.value("postcode").toString());
    ui->region->setText(rec.value("region").toString());
    ui->state->setText(rec.value("state").toString());
    ui->country->setText(rec.value("country").toString());
    ui->notes->setPlainText(rec.value("notes").toString());
}

PersonDialog::~PersonDialog()
{
    delete ui;
}

void PersonDialog::accept()
{
    if (!rec_.isEmpty()) {
        rec_.setValue("status", ui->status->currentData().toInt());
        if (is_person_) {
            rec_.setValue("gender", ui->gender->currentData().toInt());
        }
        rec_.setValue("name", ui->name->text());
        rec_.setValue("address1", ui->address1->text());
        rec_.setValue("address2", ui->address2->text());
        rec_.setValue("city", ui->city->text());
        rec_.setValue("postcode", ui->postcode->text());
        rec_.setValue("country", ui->country->text());
        rec_.setValue("region", ui->region->text());
        rec_.setValue("state", ui->state->text());
        rec_.setValue("notes", ui->notes->toPlainText());
        rec_.setValue("stars", ui->stars->currentIndex());
        rec_.setValue("favourite", ui->favorite->isChecked());

        if (rec_.value("id").toInt() > 0) {
            Q_ASSERT(row_ >= 0);
            emit updatePerson(row_, rec_);
        } else {
            emit addPerson(rec_);
            emit setFilter(rec_.value("name").toString());
        }
    }

    QDialog::accept();
}

