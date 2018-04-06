
#include <memory>

#include "src/channel.h"
#include "src/channeldialog.h"
#include "ui_channeldialog.h"

using namespace std;

ChannelDialog::ChannelDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChannelDialog)
{
    ui->setupUi(this);

    ui->type->addItem(GetChannelStatusIcon(ChannelType::OTHER), "Other",
                      static_cast<int>(ChannelType::OTHER));
    ui->type->addItem(GetChannelStatusIcon(ChannelType::WEB), "Web",
                      static_cast<int>(ChannelType::WEB));
    ui->type->addItem(GetChannelStatusIcon(ChannelType::EMAIL), "Email",
                      static_cast<int>(ChannelType::EMAIL));
    ui->type->addItem(GetChannelStatusIcon(ChannelType::PHONE), "Phone",
                      static_cast<int>(ChannelType::PHONE));
    ui->type->addItem(GetChannelStatusIcon(ChannelType::MOBILE), "Mobile",
                      static_cast<int>(ChannelType::MOBILE));
    ui->type->addItem(GetChannelStatusIcon(ChannelType::SKYPE), "Skype",
                      static_cast<int>(ChannelType::SKYPE));
    ui->type->addItem(GetChannelStatusIcon(ChannelType::LINKEDIN), "Linkedin",
                      static_cast<int>(ChannelType::LINKEDIN));
    ui->type->addItem(GetChannelStatusIcon(ChannelType::REDDIT), "Reddit",
                      static_cast<int>(ChannelType::REDDIT));
    ui->type->addItem(GetChannelStatusIcon(ChannelType::FACEBOOK), "Facebook",
                      static_cast<int>(ChannelType::FACEBOOK));
    ui->type->addItem(GetChannelStatusIcon(ChannelType::GITHUB), "Github",
                      static_cast<int>(ChannelType::GITHUB));

    ui->type->setCurrentIndex(
                ui->type->findData(static_cast<int>(ChannelType::OTHER)));
}

ChannelDialog::~ChannelDialog()
{
    delete ui;
}

void ChannelDialog::setRecord(const QSqlRecord &rec)
{
    rec_ = rec;
}

void ChannelDialog::setModel(ChannelsModel *model, QModelIndex &ix)
{
    Q_ASSERT(mapper_ == nullptr);
    Q_ASSERT(model);
    Q_ASSERT(ix.isValid());

    mapper_ = new QDataWidgetMapper(this);

    mapper_->setModel(model);
    mapper_->addMapping(ui->name, model->fieldIndex("name"));
    mapper_->addMapping(ui->type, model->fieldIndex("type"), "currentData");
    mapper_->addMapping(ui->value, model->fieldIndex("value"));
    mapper_->addMapping(ui->verified, model->fieldIndex("verified"));
    mapper_->setCurrentIndex(ix.row());

    // The mapping is not smart enough to initialize the value
    const auto dix = model->index(ix.row(), model->fieldIndex("type"), {});
    int type_val = model->data(dix, Qt::EditRole).toInt();
    ui->type->setCurrentIndex(type_val);
}


void ChannelDialog::accept()
{
    if (!rec_.isEmpty()) {

        rec_.setValue("type", ui->type->currentData().toInt());
        rec_.setValue("name", ui->name->text());
        rec_.setValue("value", ui->value->text());
        rec_.setValue("verified",
                      ui->verified->checkState() == Qt::Checked ? 1 : 0);

        emit addChannel(rec_);
    }

    if (mapper_) {
        mapper_->submit();
    }

    QDialog::accept();
}

void ChannelDialog::reject()
{
    if (mapper_) {
        mapper_->revert();
    }

    QDialog::reject();
}
