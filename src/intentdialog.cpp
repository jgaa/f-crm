#include "src/intentdialog.h"
#include "ui_intentdialog.h"
#include "intent.h"

IntentDialog::IntentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IntentDialog)
{
    ui->setupUi(this);

    for(auto e : GetIntentStateEnums()) {
         ui->state->addItem(GetIntentStateIcon(e),
                             GetIntentStateName(e),
                             static_cast<int>(e));
    }
}

IntentDialog::~IntentDialog()
{
    delete ui;
}

void IntentDialog::setRecord(const QSqlRecord &rec)
{
    rec_ = rec;
}

void IntentDialog::setModel(IntentsModel *model, QModelIndex &ix)
{
    Q_ASSERT(mapper_ == nullptr);
    Q_ASSERT(model);
    Q_ASSERT(ix.isValid());

    mapper_ = new QDataWidgetMapper(this);

    mapper_->setModel(model);
    mapper_->addMapping(ui->state, model->fieldIndex("state"), "currentData");
    mapper_->addMapping(ui->abstract, model->fieldIndex("abstract"));
    mapper_->addMapping(ui->notes, model->fieldIndex("notes"));
    mapper_->addMapping(ui->createdDate, model->fieldIndex("created_date"));

    mapper_->setCurrentIndex(ix.row());

    // The mapping is not smart enough to initialize combo boxes
    {
        const auto dix = model->index(ix.row(), model->property("state_col").toInt(), {});
        int state_val = model->data(dix, Qt::EditRole).toInt();
        ui->state->setCurrentIndex(state_val);
    }
}


void IntentDialog::accept()
{
    if (!rec_.isEmpty()) {
        rec_.setValue("state", ui->state->currentData().toInt());
        rec_.setValue("abstract", ui->abstract->text());
        rec_.setValue("notes", ui->notes->toPlainText());
        rec_.setValue("created_date", ui->createdDate->dateTime());

        emit addIntent(rec_);
    }

    if (mapper_) {
        mapper_->submit();
    }

    QDialog::accept();
}

void IntentDialog::reject()
{
    if (mapper_) {
        mapper_->revert();
    }

    QDialog::reject();
}
