
#include <QDebug>
#include <QFileDialog>
#include "src/documentdialog.h"
#include "ui_documentdialog.h"

DocumentDialog::DocumentDialog(const QSqlRecord& rec, int row, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DocumentDialog), rec_{rec}, row_{row}
{
    ui->setupUi(this);

    for(auto e : Document::typeEnums()) {
         ui->types->addItem(Document::typeIcon(e),
                            Document::typeName(e),
                            static_cast<int>(e));
    }

    for(auto e : Document::classEnums()) {
         ui->classes->addItem(Document::classIcon(e),
                              Document::className(e),
                              static_cast<int>(e));
    }

    for(auto e : Document::directionEnums()) {
         ui->direction->addItem(Document::directionIcon(e),
                                Document::directionName(e),
                                static_cast<int>(e));
    }

    for(auto e : Document::entityEnums()) {
         ui->entity->addItem(Document::entityIcon(e),
                             Document::entityName(e),
                             static_cast<int>(e));
    }

    for(int i = 0; i < rec.count(); ++i) {
        qDebug() << "# " << i << " " << rec.fieldName(i)
                 << " " << rec.value(i).typeName()
                 << " : " << (rec.isNull(i) ? QStringLiteral("NULL") : rec.value(i).toString());
    }


    ui->types->setCurrentIndex(ui->types->findData(rec.value("type").toInt()));
    ui->classes->setCurrentIndex(ui->classes->findData(rec.value("cls").toInt()));
    ui->direction->setCurrentIndex(ui->direction->findData(rec.value("direction").toInt()));
    ui->entity->setCurrentIndex(ui->entity->findData(rec.value("entity").toInt()));

    ui->name->setText(rec.value("name").toString());
    ui->location->setText(rec.value("location").toString());
    ui->notes->setPlainText(rec.value("notes").toString());

    ui->added->setDateTime(rec.value("added_date").toDateTime());

    connect(ui->entity, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onEntityCurrentIndexChanged(int)));
    connect(ui->types, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onTypeCurrentIndexChanged(int)));
    connect(ui->locationBtn, SIGNAL(clicked(bool)),
            this, SLOT(onLocationBtnClicked(bool)));
    connect(ui->openBtn, SIGNAL(clicked(bool)),
            this, SLOT(onOpenBtnClicked(bool)));


    syncEntity();
    syncType();
}

DocumentDialog::~DocumentDialog()
{
    delete ui;
}

void DocumentDialog::syncEntity()
{
    const auto entity = Document::toEntity(ui->entity->currentData().toInt());
    switch(entity) {
    case Document::Entity::CONTACT:
        fetchContacts(rec_.value("contact").toInt(), ui->currentEntityValue);
        ui->currentEntityValue->setCurrentIndex(ui->currentEntityValue->findData(rec_.value("contact").toInt()));
        break;
    case Document::Entity::PERSON:
        fetchPersons(rec_.value("contact").toInt(), ui->currentEntityValue);
        ui->currentEntityValue->setCurrentIndex(ui->currentEntityValue->findData(rec_.value("person").toInt()));
        break;
    case Document::Entity::INTENT:
        fetchIntents(rec_.value("contact").toInt(), ui->currentEntityValue);
        ui->currentEntityValue->setCurrentIndex(ui->currentEntityValue->findData(rec_.value("intent").toInt()));
        break;
    case Document::Entity::ACTION:
        fetchActions(rec_.value("contact").toInt(), ui->currentEntityValue);
        ui->currentEntityValue->setCurrentIndex(ui->currentEntityValue->findData(rec_.value("activity").toInt()));
        break;
    }
}

void DocumentDialog::syncEntityToRec()
{
    std::string col;
    switch(Document::toEntity(ui->entity->currentData().toInt())) {
    case Document::Entity::PERSON:
        col = "person";
        break;
    case Document::Entity::INTENT:
        col = "intent";
        break;
    case Document::Entity::ACTION:
        col = "activity";
        break;
    case Document::Entity::CONTACT:
        ; // Do nothing
    }

    if (!col.empty()) {
        rec_.setValue(col.c_str(), ui->currentEntityValue->currentData().toInt());
    }
}

void DocumentDialog::syncType()
{
    const auto type = Document::toType(ui->types->currentData().toInt());
    const bool enable_select = type == Document::Type::FILE;

    ui->locationBtn->setVisible(enable_select);

    ui->openBtn->setVisible(type != Document::Type::NOTE);
}

void DocumentDialog::fetchContacts(int contactId, QComboBox *combo)
{
    combo->clear();

    QSqlQuery query(
                QStringLiteral("select id, name from contact where id = %1 order by name")
                .arg(contactId));

    while(query.next()) {
        combo->addItem(query.value(1).toString(), query.value(0).toInt());
    }
}

void DocumentDialog::fetchPersons(int contactId, QComboBox *combo)
{
    combo->clear();

    QSqlQuery query(
                QStringLiteral("select id, name from contact where contact = %1 order by name")
                .arg(contactId));

    while(query.next()) {
        combo->addItem(query.value(1).toString(), query.value(0).toInt());
    }
}

void DocumentDialog::fetchIntents(int contactId, QComboBox *combo)
{
    combo->clear();

    QSqlQuery query(
                QStringLiteral("select id, abstract from intent where contact = %1 order by abstract")
                .arg(contactId));

    while(query.next()) {
        combo->addItem(query.value(1).toString(), query.value(0).toInt());
    }
}

void DocumentDialog::fetchActions(int contactId, QComboBox *combo)
{
    combo->clear();

    QSqlQuery query(
                QStringLiteral("select id, name from action where contact = %1 order by name")
                .arg(contactId));

    while(query.next()) {
        combo->addItem(query.value(1).toString(), query.value(0).toInt());
    }
}

void DocumentDialog::accept()
{
    rec_.setValue("type", ui->types->currentData().toInt());
    rec_.setValue("cls", ui->classes->currentData().toInt());
    rec_.setValue("direction", ui->direction->currentData().toInt());
    rec_.setValue("entity", ui->entity->currentData().toInt());

    rec_.setNull("person");
    rec_.setNull("intent");
    rec_.setNull("activity");

    syncEntityToRec();

    rec_.setValue("name", ui->name->text());
    rec_.setValue("location", ui->location->text());
    rec_.setValue("notes", ui->notes->toPlainText());

    rec_.setValue("added_date", ui->added->dateTime());

    if (rec_.value("id").toInt() > 0) {
        emit updateDocument(row_, rec_);
    } else {
        emit addDocument(rec_);
    }

    QDialog::accept();
}

void DocumentDialog::reject()
{
    QDialog::reject();
}

void DocumentDialog::onEntityCurrentIndexChanged(int index)
{
    Q_UNUSED(index);
    syncEntityToRec();
    syncEntity();
}

void DocumentDialog::onTypeCurrentIndexChanged(int index)
{
    Q_UNUSED(index);
    syncType();
}

void DocumentDialog::onLocationBtnClicked(bool checked)
{
    Q_UNUSED(checked);

    auto path = ui->location->text();
    auto new_path = QFileDialog::getOpenFileName(
                this, "Select File", path,{}, {},
                QFileDialog::DontResolveSymlinks);
    if (!new_path.isNull()) {
        ui->location->setText(new_path);
    }
}

void DocumentDialog::onOpenBtnClicked(bool checked)
{
    Q_UNUSED(checked);

    const auto type = Document::toType(ui->types->currentData().toInt());
    Document::open(type, ui->location->text());
}
