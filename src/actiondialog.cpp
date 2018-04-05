#include "src/actiondialog.h"
#include "ui_actiondialog.h"
#include "action.h"
#include "channel.h"
#include "utility.h"

ActionDialog::ActionDialog(const int contact, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActionDialog)
{
    static const QIcon company{":/res/icons/company.svg"};
    static const QIcon person{":/res/icons/person.svg"};

    ui->setupUi(this);

    for(auto e : GetActionStateEnums()) {
         ui->state->addItem(GetActionStateIcon(e),
                             GetActionStateName(e),
                             static_cast<int>(e));
    }

    for(auto e : GetActionTypeEnums()) {
         ui->type->addItem(GetActionTypeIcon(e),
                             GetActionTypeName(e),
                             static_cast<int>(e));
    }

    for(auto e : GetChannelTypeEnums()) {
         ui->channelType->addItem(GetChannelStatusIcon(e),
                             GetChannelTypeName(e),
                             static_cast<int>(e));
    }

    ui->person->addItem(company, "Company", 0);
    QSqlQuery persons(QStringLiteral("select id, name from contact where contact = %1 order by name")
                      .arg(contact));
    while(persons.next()) {
        ui->person->addItem(person, persons.value(1).toString(), persons.value(0).toInt());
    }


    checkAccess();

    connect(ui->type, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
}

ActionDialog::~ActionDialog()
{
    delete ui;
}

void ActionDialog::setRecord(const QSqlRecord &rec)
{
    rec_ = rec;

    ui->type->setCurrentIndex(ui->type->findData(rec.value("state").toInt()));
    ui->type->setCurrentIndex(ui->type->findData(rec.value("type").toInt()));
    ui->type->setCurrentIndex(ui->channelType->findData(rec.value("channel_type").toInt()));
    ui->type->setCurrentIndex(ui->person->findData(rec.value("person").toInt()));

    ui->name->setText(rec.value("name").toString());
    ui->goal->setPlainText(rec.value("desired_outcome").toString());
    ui->notes->setPlainText(rec.value("notes").toString());


    auto date = QDateTime::fromTime_t(rec.value("start_date").toLongLong()).date();
    ui->fromDate->setDate(date);
    ui->toTime->setDateTime(QDateTime::fromTime_t(rec.value("due_date").toLongLong()));

    checkAccess();
}

void ActionDialog::setModel(ActionsModel *model, QModelIndex &ix)
{
    Q_ASSERT(mapper_ == nullptr);
    Q_ASSERT(model);
    Q_ASSERT(ix.isValid());

    mapper_ = new QDataWidgetMapper(this);

    mapper_->setModel(model);
    mapper_->addMapping(ui->state, model->property("state_col").toInt(), "currentData");
    mapper_->addMapping(ui->type, model->property("type_col").toInt(), "currentData");
    mapper_->addMapping(ui->channelType, model->property("channel_type_col").toInt(), "currentData");
    mapper_->addMapping(ui->person, model->property("person_col").toInt(), "currentData");
    mapper_->addMapping(ui->name, model->property("name_col").toInt());
    mapper_->addMapping(ui->goal, model->property("desired_outcome_col").toInt());
    mapper_->addMapping(ui->notes, model->property("notes_col").toInt());
    mapper_->addMapping(ui->fromDate, model->property("start_date_col").toInt());
    mapper_->addMapping(ui->toTime, model->property("due_date_col").toInt());

    mapper_->setCurrentIndex(ix.row());

    // The mapping is not smart enough to initialize combo boxes
    {
        const auto dix = model->index(ix.row(), model->property("state_col").toInt(), {});
        int val = model->data(dix, Qt::DisplayRole).toInt();
        const auto state = ui->state->findData(val);
        ui->state->setCurrentIndex(state);
    }

    {
        const auto dix = model->index(ix.row(), model->fieldIndex("type"), {});
        int val = model->data(dix, Qt::DisplayRole).toInt();
        ui->type->setCurrentIndex(ui->type->findData(val));
    }

    {
        const auto dix = model->index(ix.row(), model->fieldIndex("channel_type"), {});
        int val = model->data(dix, Qt::DisplayRole).toInt();
        ui->channelType->setCurrentIndex(ui->channelType->findData(val));
    }

    {
        const auto dix = model->index(ix.row(), model->fieldIndex("person"), {});
        int val = model->data(dix, Qt::EditRole).toInt();
        ui->person->setCurrentIndex(ui->person->findData(val));
    }

    checkAccess();
}

void ActionDialog::checkAccess()
{
    if (ui->type->currentData().toInt() == static_cast<int>(ActionType::CHANNEL)) {
        ui->channelType->setEnabled(true);
        if (ui->channelType->currentIndex() < 0) {
            ui->channelType->setCurrentIndex(0);
        }
    } else {
        ui->channelType->setCurrentIndex(-1);
        ui->channelType->setEnabled(false);
    }
}


void ActionDialog::accept()
{
    if (!rec_.isEmpty()) {
        rec_.setValue("state", ui->state->currentData().toInt());
        rec_.setValue("type", ui->type->currentData().toInt());
        rec_.setValue("channel_type", ui->channelType->currentData().toInt());

        if (ui->person->currentData().toInt() <= 0) {
            rec_.setNull("person");
        } else {
            rec_.setValue("person", ui->person->currentData().toInt());
        }

        rec_.setValue("name", ui->name->text());
        rec_.setValue("desired_outcome", ui->goal->toPlainText());
        rec_.setValue("notes", ui->notes->toPlainText());

        rec_.setValue("start_date", static_cast<uint>(ToTime(ui->fromDate->date())));
        rec_.setValue("due_date", static_cast<uint>(ui->toTime->dateTime().toTime_t()));

        emit addAction(rec_);
    }

    if (mapper_) {
        mapper_->submit();
    }

    QDialog::accept();
}

void ActionDialog::reject()
{
    if (mapper_) {
        mapper_->revert();
    }

    QDialog::reject();
}

void ActionDialog::onCurrentIndexChanged(int index)
{
    Q_UNUSED(index);
    checkAccess();
}
