#include "channeldialog.h"
#include "mainwindow.h"
#include "persondialog.h"
#include "intentdialog.h"
#include "ui_mainwindow.h"
#include "intent.h"
#include "action.h"
#include "actiondialog.h"
#include "actionexecutedialog.h"
#include "documentdialog.h"


#include <QSettings>
#include <QDebug>
#include <QSqlRecord>
#include <QClipboard>
#include <QDesktopServices>
#include <QMessageBox>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
    QIcon appicon(":res/icons/f-crm.svg");
    setWindowIcon(appicon);

    if (settings_.value("restore-window-state", true).toBool()) {
        restoreGeometry(settings_.value("windowGeometry").toByteArray());
        restoreState(settings_.value("windowState").toByteArray());
    }

    db_ = std::make_unique<Database>(nullptr);

    log_model_ = new LogModel(settings_, this, {});
    log_px_model_ = new LogProxyModel(log_model_, this);
    ui->logView->setModel(log_px_model_);

    contacts_model_ = new ContactsModel(settings_, this, {});
    contact_px_model = new ContactProxyModel(contacts_model_, this);
    persons_model_ = new ContactsModel(settings_, this, {});
    persons_model_->setParent(-1);
    person_px_model = new ContactProxyModel(persons_model_, this);
    channels_model_ = new ChannelsModel(settings_, this, {});
    channels_px_model_ = new ChannelProxyModel(channels_model_, this);
    intents_model_ = new IntentsModel(settings_, this, {});
    intents_px_model_ = new IntentProxyModel(intents_model_, this);
    actions_model_ = new ActionsModel(settings_, this, {});
    actions_px_model_ = new ActionProxyModel(actions_model_, this);
    documents_model_ = new DocumentsModel(settings_, this, {});
    documents_px_model_ = new DocumentProxyModel(documents_model_, this);

    ui->contactsList->setModel(contact_px_model);
    ui->contactsList->setDocumentsModel(documents_model_);
    ui->contactsList->setEntity(Document::Entity::CONTACT, contacts_model_, 0);
    ui->contactsList->setDocumentDropEnabled(true);

    contacts_model_->select();

    {
        auto home = new QListWidgetItem(QIcon(":/res/icons/home.svg"), QStringLiteral("Panel"), nullptr, 0);
        home->setToolTip(QStringLiteral("Home screen"));
        ui->appModeList->addItem(home);

        auto contacts = new QListWidgetItem(QIcon(":/res/icons/users.svg"), QStringLiteral("Contacts"), nullptr, 0);
        contacts->setToolTip(QStringLiteral("Contacts screen"));
        ui->appModeList->addItem(contacts);

        connect(ui->appModeList, &QListWidget::itemSelectionChanged,
                this, &MainWindow::appModeSelectionChanged);

        ui->appModeList->setCurrentItem(home);
    }

    ui->contactsList->horizontalHeader()->setSectionResizeMode(
                contacts_model_->property("name_col").toInt(), QHeaderView::Stretch);
    for(int i = 0; i < contacts_model_->columnCount(); ++i) {
        const bool show =
                (i == contacts_model_->property("name_col").toInt())
                || (i == contacts_model_->property("status_col").toInt());
        ui->contactsList->setColumnHidden(i, !show);
    }

    ui->contactChannels->setModel(channels_px_model_);

    ui->contactChannels->horizontalHeader()->setSectionResizeMode(
                channels_model_->property("value_col").toInt(), QHeaderView::Stretch);
    for(int i = 0; i < channels_model_->columnCount(); ++i) {
        const bool show = (i == channels_model_->property("value_col").toInt())
                || (i == channels_model_->property("verified_col").toInt());
        ui->contactChannels->setColumnHidden(i, !show);
    }

    ui->contactPeople->setModel(person_px_model);
    ui->contactPeople->setDocumentsModel(documents_model_);
    ui->contactPeople->horizontalHeader()->setSectionResizeMode(
                persons_model_->property("name_col").toInt(), QHeaderView::Stretch);
    for(int i = 0; i < persons_model_->columnCount(); ++i) {
        const bool show =
                (i == persons_model_->property("name_col").toInt())
                || (i == persons_model_->property("status_col").toInt());
        ui->contactPeople->setColumnHidden(i, !show);
    }

    ui->intentsView->setModel(intents_px_model_);
    ui->intentsView->setDocumentsModel(documents_model_);

    ui->intentsView->horizontalHeader()->setSectionResizeMode(
                intents_model_->property("abstract_col").toInt(), QHeaderView::Stretch);
    for(int i = 0; i < intents_model_->columnCount(); ++i) {
        const bool show = (i == intents_model_->property("abstract_col").toInt())
                || (i == intents_model_->property("state_col").toInt());
        ui->intentsView->setColumnHidden(i, !show);
    }

    ui->actionsView->setModel(actions_px_model_);
    ui->actionsView->setDocumentsModel(documents_model_);
    ui->actionsView->horizontalHeader()->setSectionResizeMode(
                actions_model_->property("name_col").toInt(), QHeaderView::Stretch);
    ui->actionsView->horizontalHeader()->moveSection(actions_model_->property("start_date_col").toInt(), 0);
    for(int i = 0; i < actions_model_->columnCount(); ++i) {
        const bool show = (i == actions_model_->property("name_col").toInt())
                || (i == actions_model_->property("state_col").toInt())
                || (i == actions_model_->property("start_date_col").toInt())
                || (i == actions_model_->property("person_col").toInt());
        ui->actionsView->setColumnHidden(i, !show);
    }

    ui->logView->horizontalHeader()->setSectionResizeMode(
                log_model_->property("text_col").toInt(), QHeaderView::Stretch);
    for(int i = 0; i < log_model_->columnCount(); ++i) {
        const bool show = (i == log_model_->property("text_col").toInt())
                || (i == log_model_->property("date_col").toInt());
        ui->logView->setColumnHidden(i, !show);
    }

    ui->documentsView->setModel(documents_px_model_);
    ui->documentsView->setDocumentsModel(documents_model_);
    ui->documentsView->horizontalHeader()->setSectionResizeMode(
                documents_model_->property("name_col").toInt(), QHeaderView::Stretch);
    for(int i = 0; i < documents_model_->columnCount(); ++i) {
        const bool show = (i == documents_model_->property("cls_col").toInt())
                || (i == documents_model_->property("direction_col").toInt())
                || (i == documents_model_->property("entity_col").toInt())
                || (i == documents_model_->property("name_col").toInt())
                || (i == documents_model_->property("added_date_col").toInt());
        ui->documentsView->setColumnHidden(i, !show);
    }
    ui->documentsView->horizontalHeader()->moveSection(documents_model_->property("added_date_col").toInt(), 0);


    ui->contactTab->setCurrentIndex(0);


    connect(ui->contactFilter, &QLineEdit::textChanged, this, &MainWindow::onContactFilterChanged);
    connect(ui->contactsList, &QTableView::activated, this, &MainWindow::onContactsListRowActivated);
    connect(ui->contactsList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onContactsListSelectionChanged);
    connect(ui->contactsList->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onContactsListCurrentChanged);
    connect(contacts_model_, &ContactsModel::modelReset, this, &MainWindow::onContactsModelReset);
    connect(ui->contactsList, &QTableView::customContextMenuRequested,
            this, &MainWindow::onContactContextMenuRequested);

    connect(ui->contactChannels->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onChannelListRowActivated);
    connect(ui->contactChannels, &QTableView::customContextMenuRequested,
            this, &MainWindow::onChannelContextMenuRequested);
    connect(channels_model_, &ChannelsModel::modelReset,
            this, &MainWindow::onChannelsModelReset);

    connect(ui->contactPeople->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onPersonsListRowActivated);
    connect(ui->contactPeople, &QTableView::customContextMenuRequested,
            this, &MainWindow::onPersonsContextMenuRequested);
    connect(ui->contactPeople, &QTableView::clicked,
            this, &MainWindow::onPersonsClicked);
    connect(persons_model_, &ContactsModel::modelReset,
            this, &MainWindow::onPersonsModelReset);

    connect(ui->intentsView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onIntentsRowActivated);
    connect(ui->intentsView, &QTableView::customContextMenuRequested,
            this, &MainWindow::onIntentsContextMenuRequested);
    connect(intents_model_, &IntentsModel::modelReset,
            this, &MainWindow::onIntentsModelReset);

    connect(ui->actionsView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onActionsRowActivated);
    connect(ui->actionsView, &QTableView::customContextMenuRequested,
            this, &MainWindow::onActionsContextMenuRequested);
    connect(actions_model_, &ActionsModel::modelReset,
            this, &MainWindow::onActionsModelReset);
    connect(actions_model_, &ActionsModel::dataChanged,
            this, &MainWindow::onActionsDataChanged);

    connect(ui->documentsView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onDocumentsRowActivated);
    connect(ui->documentsView, &QTableView::customContextMenuRequested,
            this, &MainWindow::onDocumentsContextMenuRequested);
    connect(documents_model_, &DocumentsModel::modelReset,
            this, &MainWindow::onDocumentsModelReset);
    connect(documents_model_, &DocumentsModel::dataChanged,
            this, &MainWindow::onDocumentsDataChanged);


    connect(ui->contactTab, &QTabWidget::currentChanged, this, &MainWindow::onContactTabChanged);

    onSyncronizeContactsBindings();
}

void MainWindow::appModeSelectionChanged()
{
    static const array<AppMode,2> modes = {{AppMode::PANEL, AppMode::CONTACTS}};

    const int selection = ui->appModeList->currentRow();
    ui->stackedWidget->setCurrentIndex(selection);

    app_mode_ = modes.at(static_cast<size_t>(selection));

    onSyncronizeContactsBindings();
    onValidateChannelActions();
    onValidateContactActions();
    onValidatePersonsActions();
}

void MainWindow::onContactFilterChanged(const QString &text)
{
    // We can't use filter() - it don't handle special characters.
    contacts_model_->setNameFilter(text);
}

void MainWindow::onContactsListRowActivated(const QModelIndex &ix)
{
    Q_UNUSED(ix);
}


void MainWindow::onContactsListSelectionChanged(const QItemSelection &selected,
                                                const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    const auto current = ui->contactsList->currentIndex();

    qDebug() << "Selection changed: Current item is" << (current.isValid() ? current.row() : -1);
}

void MainWindow::onContactsListCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    qDebug() << "Current changed: Current item is" << (current.isValid() ? current.row() : -1);

    onSyncronizeContactsBindings();
}

void MainWindow::onContactsModelReset()
{
    const auto current = ui->contactsList->currentIndex();

    qDebug() << "Contact model was reset: Current item is" << (current.isValid() ? current.row() : -1);

    onSyncronizeContactsBindings();
}

void MainWindow::onSyncronizeContactsBindings()
{
    const auto current = ui->contactsList->currentIndex();

    qDebug() << "onSyncronizeContactsBindings: Current item is" << (current.isValid() ? current.row() : -1);

    bool read_only = true;

    if (current.isValid()) {
        if (!contacts_mapper_) {
            contacts_mapper_ = make_unique<QDataWidgetMapper>();
            contacts_mapper_->setModel(contacts_model_);
            contacts_mapper_->addMapping(ui->contactNotes, contacts_model_->property("notes_col").toInt());

        } else {
            if (contacts_mapper_->currentIndex() != current.row()) {
                contacts_mapper_->setCurrentIndex(current.row());
            }
        }

        read_only = false;
        const auto contact_id = contacts_model_->getContactId(current);

        const auto contact_type = contacts_model_->data(
                    contacts_model_->index(current.row(),
                                           contacts_model_->property("type_col").toInt()),
                    Qt::DisplayRole).toInt();

        if (contact_type == static_cast<int>(ContactType::CORPORATION)) {
            ui->contactPeople->setEnabled(true);
            persons_model_->setParent(contact_id);
            persons_model_->select();
        } else {
            ui->contactPeople->setEnabled(false);
            persons_model_->setParent(-1);
            persons_model_->select();
        }

        ui->contactType->setPixmap(GetContactTypeIcon(contact_type).pixmap(32,32));

        onSyncronizePersonBindings();

        intents_model_->setContact(contact_id);
        actions_model_->setContact(contact_id);
        log_model_->setContact(contact_id);
        documents_model_->setContact(contact_id);
        ui->documentsView->setContactId(contact_id);
        ui->documentsView->setEntity(Document::Entity::CONTACT, nullptr, contact_id);
        ui->documentsView->setDocumentDropEnabled(true);
        ui->contactPeople->setContactId(contact_id);
        ui->contactPeople->setEntity(Document::Entity::PERSON, persons_model_, -1);
        ui->contactPeople->setDocumentDropEnabled(true);

        ui->intentsView->setContactId(contact_id);
        ui->intentsView->setEntity(Document::Entity::INTENT, intents_model_, -1);
        ui->intentsView->setDocumentDropEnabled(true);

        ui->actionsView->setContactId(contact_id);
        ui->actionsView->setEntity(Document::Entity::ACTION, actions_model_, -1);
        ui->actionsView->setDocumentDropEnabled(true);
    } else {
        if (contacts_mapper_) {
            contacts_mapper_->setCurrentIndex(-1);
        }
        ui->contactNotes->clear();
        ui->contactAddress->clear();
        ui->contactAddress2->clear();
        ui->contactCity->clear();
        ui->contactPostCode->clear();
        ui->contactCountry->clear();

        channels_model_->setContact(-1);
        ui->contactPeople->setEnabled(false);

        persons_model_->setParent(-1);
        persons_model_->select();

        ui->contactType->setPixmap({});

        ui->personWhoIcon->setPixmap({});
        ui->personWhoName->setText({});

        actions_model_->setContact(-1);
        intents_model_->setContact(-1);
        log_model_->setContact(-1);
        documents_model_->setContact(-1);
        ui->documentsView->setContactId(-1);
        ui->documentsView->setDocumentDropEnabled(false);
        ui->contactPeople->setDocumentDropEnabled(false);
        ui->intentsView->setDocumentDropEnabled(false);
        ui->actionsView->setDocumentDropEnabled(true);
    }

    ui->contactNotes->setReadOnly(read_only);
    ui->contactAddress->setReadOnly(read_only);
    ui->contactAddress2->setReadOnly(read_only);
    ui->contactCity->setReadOnly(read_only);
    ui->contactPostCode->setReadOnly(read_only);
    ui->contactCountry->setReadOnly(read_only);

    last_person_clicked = -1;

    onValidateContactActions();
    onValidatePersonsActions();
}

void MainWindow::onSyncronizePersonBindings()
{
    const auto current = ui->contactsList->currentIndex();

    if (contacts_mapper_) {
        contacts_mapper_->submit();
    }

    if (persons_mapper_) {
        persons_mapper_->submit();
    }

    if (current.isValid() && contacts_mapper_) {

        const auto people = ui->contactPeople->currentIndex();
        if (people.isValid()) {
            // Make sure properties are mapped to person
            if (!persons_mapper_) {
                contacts_mapper_->removeMapping(ui->contactAddress);
                contacts_mapper_->removeMapping(ui->contactAddress2);
                contacts_mapper_->removeMapping(ui->contactCity);
                contacts_mapper_->removeMapping(ui->contactPostCode);
                contacts_mapper_->removeMapping(ui->contactCountry);
                contacts_mapper_->removeMapping(ui->personNotes);

                persons_mapper_ = make_unique<QDataWidgetMapper>();
                persons_mapper_->setModel(persons_model_);

                persons_mapper_->addMapping(ui->contactAddress, persons_model_->property("address1_col").toInt());
                persons_mapper_->addMapping(ui->contactAddress2, persons_model_->property("address2_col").toInt());
                persons_mapper_->addMapping(ui->contactCity, persons_model_->property("city_col").toInt());
                persons_mapper_->addMapping(ui->contactPostCode, persons_model_->property("postcode_col").toInt());
                persons_mapper_->addMapping(ui->contactCountry, persons_model_->property("country_col").toInt());
                persons_mapper_->addMapping(ui->personNotes, persons_model_->property("notes_col").toInt());

                persons_mapper_->setCurrentIndex(people.row());
            } else {
                if (persons_mapper_->currentIndex() != people.row()) {
                    persons_mapper_->setCurrentIndex(people.row());
                }
            }

            contacts_mapper_->setCurrentIndex(current.row());
            const auto contact_id = persons_model_->getContactId(people);
            channels_model_->setContact(contact_id);

            auto whoix = persons_model_->index(people.row(), persons_model_->property("name_col").toInt(), {});
            ui->personWhoIcon->setPixmap(persons_model_->data(whoix, Qt::DecorationRole).value<QIcon>().pixmap(24,24));
            ui->personWhoName->setText(persons_model_->data(whoix, Qt::DisplayRole).toString());
        } else {
            // Make sure properties are mapped to company (parent)
            if (persons_mapper_) {
                persons_mapper_.reset();

                contacts_mapper_->addMapping(ui->contactAddress, contacts_model_->property("address1_col").toInt());
                contacts_mapper_->addMapping(ui->contactAddress2, contacts_model_->property("address2_col").toInt());
                contacts_mapper_->addMapping(ui->contactCity, contacts_model_->property("city_col").toInt());
                contacts_mapper_->addMapping(ui->contactPostCode, contacts_model_->property("postcode_col").toInt());
                contacts_mapper_->addMapping(ui->contactCountry, contacts_model_->property("country_col").toInt());
                contacts_mapper_->addMapping(ui->personNotes, contacts_model_->property("notes_col").toInt());
            }

            contacts_mapper_->setCurrentIndex(current.row());
            const auto contact_id = contacts_model_->getContactId(current);
            channels_model_->setContact(contact_id);

            auto whoix = contacts_model_->index(current.row(), contacts_model_->property("name_col").toInt(), {});
            ui->personWhoIcon->setPixmap(contacts_model_->data(whoix, Qt::DecorationRole).value<QIcon>().pixmap(24,24));
            ui->personWhoName->setText(contacts_model_->data(whoix, Qt::DisplayRole).toString());
        }
    }
}

void MainWindow::onValidateContactActions()
{
    const auto current = ui->contactsList->currentIndex();
    const bool enabled = app_mode_ == AppMode::CONTACTS;
    const bool enable_modifications = enabled && current.isValid();

    ui->actionAdd_Contact->setEnabled(enabled);
    ui->actionAdd_Company->setEnabled(enabled);

    ui->actionDelete_Contact->setEnabled(enable_modifications);
}

void MainWindow::onContactContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu;

    menu->addAction(ui->actionAdd_Company);
    menu->addAction(ui->actionAdd_Contact);
    menu->addAction(ui->actionDelete_Contact);
    menu->addSeparator();
    menu->addAction(ui->actionAdd_Person);
    menu->addAction(ui->actionEdit_Person);
    menu->addAction(ui->actionDelete_Person);

    menu->exec(ui->contactsList->mapToGlobal(pos));
}

void MainWindow::onChannelContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu;

    menu->addAction(ui->actionAdd_Channel);
    menu->addAction(ui->actionEdit_Channel);
    menu->addAction(ui->actionVerify_Channel);
    menu->addAction(ui->actionDelete_Channel);
    menu->addSeparator();
    menu->addAction(ui->actionCopy_Channel_to_Clipboard);
    menu->addAction(ui->actionOpen_Channel);

    menu->exec(ui->contactChannels->mapToGlobal(pos));
}

void MainWindow::onChannelListRowActivated(const QModelIndex &index)
{
    Q_UNUSED(index);
    onValidateChannelActions();
}

void MainWindow::onChannelsModelReset()
{
    onValidateChannelActions();
}

void MainWindow::onValidateChannelActions()
{
    const auto current = persons_mapper_ ? ui->contactPeople->currentIndex() : ui->contactsList->currentIndex();
    const bool enable_channel = current.isValid
            () && app_mode_ == AppMode::CONTACTS
            && ui->contactTab->currentIndex() == static_cast<int>(PersonTab::CONTACT);
    bool enable_modifications = false;
    bool enable_open = false;

    if (enable_channel) {
        const auto current_ch = ui->contactChannels->currentIndex();
        enable_modifications = current_ch.isValid();

        if (enable_modifications) {
            const auto type = getChannelType();

            switch(type) {
            case ChannelType::MOBILE:
            case ChannelType::PHONE:
                break;
            case ChannelType::EMAIL:
            case ChannelType::SKYPE:
                enable_open = true;
                break;
            default: {
                const auto value = getChannelValue();
                if (value.startsWith("http://") || value.startsWith("https://")) {
                    enable_open = true;
                }
            } // default
            } // switch
        }
    }

    ui->actionAdd_Channel->setEnabled(enable_channel);
    ui->actionDelete_Channel->setEnabled(enable_modifications);
    ui->actionEdit_Channel->setEnabled(enable_modifications);
    ui->actionVerify_Channel->setEnabled(enable_modifications);
    ui->actionCopy_Channel_to_Clipboard->setEnabled(enable_modifications);
    ui->actionOpen_Channel->setEnabled(enable_open);
}

void MainWindow::onPersonsContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu;

    menu->addAction(ui->actionAdd_Person);
    menu->addAction(ui->actionEdit_Person);
    menu->addAction(ui->actionDelete_Person);

    menu->exec(ui->contactPeople->mapToGlobal(pos));
}

void MainWindow::onPersonsListRowActivated(const QModelIndex &index)
{
    Q_UNUSED(index);

    onSyncronizePersonBindings();
    onValidatePersonsActions();

}

void MainWindow::onPersonsModelReset()
{
    onValidatePersonsActions();
    onSyncronizePersonBindings();
}

void MainWindow::onPersonsClicked(const QModelIndex &index)
{

    // Toggle selection
    if (last_person_clicked == index.row()) {
        ui->contactPeople->setCurrentIndex({});
        onSyncronizePersonBindings();
        last_person_clicked = -1;
    } else {
        last_person_clicked = index.row();
    }

}

void MainWindow::onValidatePersonsActions()
{
    const auto current = ui->contactsList->currentIndex();
    const bool enable = current.isValid() && app_mode_ == AppMode::CONTACTS;
    bool enable_modifications = false;

    ui->actionAdd_Person->setEnabled(enable);

    if (enable) {
        const auto current_ch = ui->contactPeople->currentIndex();
        enable_modifications = current_ch.isValid();
    }

    ui->actionEdit_Person->setEnabled(enable_modifications);
    ui->actionDelete_Person->setEnabled(enable_modifications);
}

void MainWindow::onIntentsContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu;

    menu->addAction(ui->actionAdd_Intent);
    menu->addAction(ui->actionEdit_Intent);
    menu->addAction(ui->actionDelete_Intent);

    menu->exec(ui->intentsView->mapToGlobal(pos));
}

void MainWindow::onIntentsRowActivated(const QModelIndex &index)
{
    Q_UNUSED(index);
    onValidateIntentActions();

    actions_model_->setIntent(intents_model_->getIntentId(ui->intentsView->currentIndex()));

    onValidateActionActions();
}

void MainWindow::onIntentsModelReset()
{
    onValidateIntentActions();

    actions_model_->setIntent(-1);

    onValidateActionActions();
}

void MainWindow::onValidateIntentActions()
{
    const auto current = ui->contactsList->currentIndex();
    const bool enable = current.isValid()
            && app_mode_ == AppMode::CONTACTS
            && ui->contactTab->currentIndex() == static_cast<int>(PersonTab::INTENTS);

    ui->actionAdd_Intent->setEnabled(enable);
    bool enable_modifications = false;

    if (enable) {
        const auto current_intent = ui->intentsView->currentIndex();
        enable_modifications = current_intent.isValid();
    }

    ui->actionEdit_Intent->setEnabled(enable_modifications);
    ui->actionDelete_Intent->setEnabled(enable_modifications);
}

void MainWindow::onActionsContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu;

    menu->addAction(ui->actionAdd_Action);
    menu->addAction(ui->actionEdit_Action);
    menu->addAction(ui->actionDelete_Action);
    menu->addSeparator();
    menu->addAction(ui->actionMove_Action_Up);
    menu->addAction(ui->actionMove_Action_Down);
    menu->addAction(ui->actionAction_Done);
    menu->addSeparator();
    menu->addAction(ui->actionExecute_Action);
    menu->addAction(ui->actionAction_Done);

    menu->exec(ui->actionsView->mapToGlobal(pos));
}

void MainWindow::onActionsRowActivated(const QModelIndex &index)
{
    Q_UNUSED(index)
    onValidateActionActions();
}

void MainWindow::onActionsDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)
{
    onValidateActionActions();
}

void MainWindow::onActionsModelReset()
{
    onValidateActionActions();
}

void MainWindow::onValidateActionActions()
{
    const auto current = ui->contactsList->currentIndex();
    const bool enable = current.isValid()
            && app_mode_ == AppMode::CONTACTS
            && ui->contactTab->currentIndex() == static_cast<int>(PersonTab::INTENTS)
            && ui->intentsView->currentIndex().isValid();

    ui->actionAdd_Action->setEnabled(enable);
    bool enable_modifications = false;
    bool enable_completion = false;

    if (enable) {
        const auto current_action = ui->actionsView->currentIndex();
        enable_modifications = current_action.isValid();

        if (enable_modifications) {

            const auto aix = actions_model_->index(current_action.row(),
                                                   actions_model_->property("state_col").toInt(), {});
            const auto state = actions_model_->data(aix, Qt::DisplayRole).toInt();
            enable_completion = state < static_cast<int>(ActionState::DONE);
        }
    }

    ui->actionEdit_Action->setEnabled(enable_modifications);
    ui->actionDelete_Action->setEnabled(enable_modifications);
    ui->actionMove_Action_Up->setEnabled(enable_modifications);
    ui->actionMove_Action_Down->setEnabled(enable_modifications);

    // Is the action open?
    ui->actionExecute_Action->setEnabled(enable_completion);
    ui->actionAction_Done->setEnabled(enable_completion);

}

void MainWindow::onDocumentsContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    QMenu *menu = new QMenu;

    menu->addAction(ui->actionAdd_Document);
    menu->addAction(ui->actionEdit_Document);
    menu->addAction(ui->actionDelete_Document);
    menu->addSeparator();
    menu->addAction(ui->actionOpen_Document);

    menu->exec(ui->documentsView->mapToGlobal(pos));
}

void MainWindow::onDocumentsRowActivated(const QModelIndex &index)
{
    Q_UNUSED(index);
    onValidateDocumentActions();
}

void MainWindow::onDocumentsDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)
{
    onValidateDocumentActions();
}

void MainWindow::onDocumentsModelReset()
{
    onValidateDocumentActions();
}

void MainWindow::onValidateDocumentActions()
{
    const auto current = ui->contactsList->currentIndex();
    const bool enable = current.isValid()
            && app_mode_ == AppMode::CONTACTS
            && ui->contactTab->currentIndex() == static_cast<int>(PersonTab::DOCUMENTS);

    ui->actionAdd_Document->setEnabled(enable);
    bool enable_modifications = false;

    if (enable) {
        const auto current_intent = ui->documentsView->currentIndex();
        enable_modifications = current_intent.isValid();
    }

    ui->actionEdit_Document->setEnabled(enable_modifications);
    ui->actionDelete_Document->setEnabled(enable_modifications);
    ui->actionOpen_Document->setEnabled(enable_modifications);
}

void MainWindow::onContactTabChanged(int ix)
{
    Q_UNUSED(ix);
    onValidateChannelActions();
    onValidatePersonsActions();
    onValidateContactActions();
    onValidateIntentActions();
    onValidateActionActions();
    onValidateDocumentActions();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    settings_.setValue("windowGeometry", saveGeometry());
    settings_.setValue("windowState", saveState());
}

void MainWindow::on_action_Quit_triggered()
{
    this->close();
}

void MainWindow::on_actionDelete_Contact_triggered()
{
    auto selected = ui->contactsList->selectionModel()->selection().indexes();

    if (selected.isEmpty()) {
        return;
    }

    if (!confirmDelete("Contact")) {
        return;
    }

    ui->contactsList->setCurrentIndex({});
    contacts_model_->removeContacts(selected);
}

void MainWindow::on_actionAdd_Channel_triggered()
{
    if (const auto contact_id = getCurrentPersonId()) {
        auto rec = channels_model_->record();

        rec.setValue("contact", contact_id);

        auto dlg = new ChannelDialog(this);
        dlg->setRecord(rec);
        dlg->setAttribute( Qt::WA_DeleteOnClose );
        connect(dlg, &ChannelDialog::addChannel,
                channels_model_,
                &ChannelsModel::addChannel);
        dlg->exec();
    }
}

void MainWindow::on_actionDelete_Channel_triggered()
{
    auto selected = ui->contactChannels->selectionModel()->selection().indexes();

    if (selected.isEmpty()) {
        return;
    }

    if (!confirmDelete("Channel")) {
        return;
    }

    ui->contactChannels->setCurrentIndex({});
    channels_model_->removeChannels(selected);
}

void MainWindow::on_actionEdit_Channel_triggered()
{
    auto current = ui->contactChannels->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return;
    }

    auto dlg = new ChannelDialog(this);
    dlg->setModel(channels_model_, current);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->exec();
}

void MainWindow::on_actionVerify_Channel_triggered()
{
    auto selected = ui->contactChannels->selectionModel()->selection().indexes();

    if (selected.isEmpty()) {
        return;
    }

    ui->contactChannels->setCurrentIndex({});
    channels_model_->verifyChannels(selected);
}

void MainWindow::on_actionCopy_Channel_to_Clipboard_triggered()
{
    auto value = getChannelValue();
    if (!value.isEmpty()) {
        auto clipboard = QGuiApplication::clipboard();
        clipboard->setText(value);
    }
}

void MainWindow::on_actionOpen_Channel_triggered()
{
    openChannel(getChannelType(), getChannelValue());
}

void MainWindow::openChannel(const ChannelType type, const QString& value) {
    if (value.isEmpty()) {
        return;
    }

    switch(type) {
    case ChannelType::EMAIL:
        QDesktopServices::openUrl({QStringLiteral("mailto:%1").arg(value)});
    break;
    case ChannelType::SKYPE:
        QDesktopServices::openUrl({QStringLiteral("skype:%1").arg(value)});
    break;
    case ChannelType::PHONE:
    case ChannelType::MOBILE:
        ; // Do nothing
    break;
    default:
        if (value.startsWith("http://") || value.startsWith("https://")) {
            QDesktopServices::openUrl({value});
        }
    }
}

bool MainWindow::confirmDelete(const QString &what)
{
    return QMessageBox::warning(this, "You are about to delete information",
                                QStringLiteral("Do you really want to delete this %1?")
                                .arg(what),
                                QMessageBox::Yes,
                                QMessageBox::No | QMessageBox::Default)
            == QMessageBox::Yes;
}

QString MainWindow::getChannelValue() const
{
    auto current = ui->contactChannels->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return {};
    }

    const auto ix = channels_model_->index(current.row(),
                                           channels_model_->property("value_col").toInt(),
                                           {});
    return channels_model_->data(ix, Qt::EditRole).toString();
}

ChannelType MainWindow::getChannelType() const
{
    auto current = ui->contactChannels->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return ChannelType::OTHER;
    }

    const auto ix = channels_model_->index(current.row(),
                                           channels_model_->property("type_col").toInt(),
                                           {});
    return ToChannelType(channels_model_->data(ix, Qt::EditRole).toInt());
}


void MainWindow::on_actionAdd_Person_triggered()
{
    auto current = ui->contactsList->selectionModel()->currentIndex();

    if (!current.isValid()) {
        return;
    }

    const auto contact_id = contacts_model_->getContactId(current);

    auto rec = persons_model_->record();
    rec.setValue("contact", contact_id);
    rec.setValue("type", static_cast<int>(ContactType::INDIVID));

    auto dlg = new PersonDialog(this);
    dlg->setRecord(rec);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect(dlg, &PersonDialog::addPerson,
            persons_model_, &ContactsModel::addPerson);
    dlg->exec();
}

void MainWindow::on_actionAdd_Company_triggered()
{
    createContact(ContactType::CORPORATION);
}

void MainWindow::on_actionAdd_Contact_triggered()
{
    createContact(ContactType::INDIVID);
}

void MainWindow::createContact(ContactType type)
{
    if (!ui->contactFilter->text().isEmpty()) {
        ui->contactFilter->setText("");
    }
    const auto ix = contacts_model_->createContact(type);
    if (ix.isValid()) {
        ui->contactsList->setFocus();
        ui->contactsList->setCurrentIndex(ix);
        onSyncronizeContactsBindings();
        ui->contactsList->edit(ix);
    }
}

bool MainWindow::isCurrentPersonInCompany() const
{
    return persons_mapper_ != nullptr;
}

QTableView *MainWindow::getCurrentPersonView() const
{
    return isCurrentPersonInCompany() ? ui->contactPeople : ui->contactsList;
}

QModelIndex MainWindow::getCurrentPersonIndex() const
{
    return getCurrentPersonView()->selectionModel()->currentIndex();
}

ContactsModel *MainWindow::getCurrentPersonModel() const
{
    return isCurrentPersonInCompany() ? persons_model_ : contacts_model_;
}

int MainWindow::getCurrentPersonId() const
{
    const auto current = getCurrentPersonIndex();
    if (!current.isValid()) {
        return 0;
    }

    return getCurrentPersonModel()->getContactId(current);
}


void MainWindow::on_actionEdit_Person_triggered()
{
    auto current = ui->contactPeople->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return;
    }

    auto dlg = new PersonDialog(this);
    dlg->setModel(persons_model_, current);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->exec();
}

void MainWindow::on_actionDelete_Person_triggered()
{
    auto selected = ui->contactPeople->selectionModel()->selection().indexes();

    if (selected.isEmpty()) {
        return;
    }

    if (!confirmDelete("Person")) {
        return;
    }

    ui->contactPeople->setCurrentIndex({});
    persons_model_->removeContacts(selected);
}

void MainWindow::on_actionAdd_Intent_triggered()
{
    const auto current = ui->contactsList->currentIndex();

    if (!current.isValid()) {
        return;
    }

    const auto contact_id = contacts_model_->getContactId(current);

    auto rec = intents_model_->record();
    rec.setValue("contact", contact_id);
    rec.setValue("type", static_cast<int>(IntentType::MANUAL));

    auto dlg = new IntentDialog(this);
    dlg->setRecord(rec);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect(dlg, &IntentDialog::addIntent,
            intents_model_, &IntentsModel::addIntent);
    dlg->exec();
    intents_model_->select();
}

void MainWindow::on_actionEdit_Intent_triggered()
{
    auto current = ui->intentsView->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return;
    }

    auto dlg = new IntentDialog(this);
    dlg->setModel(intents_model_, current);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->exec();
}

void MainWindow::on_actionDelete_Intent_triggered()
{
    auto selected = ui->intentsView->selectionModel()->selection().indexes();

    if (selected.isEmpty()) {
        return;
    }

    if (!confirmDelete("Intent")) {
        return;
    }

    ui->intentsView->setCurrentIndex({});
    intents_model_->removeIntents(selected);
}



void MainWindow::on_actionAdd_Action_triggered()
{
    const auto current = ui->intentsView->currentIndex();

    if (!current.isValid()) {
        return;
    }

    auto rec = actions_model_->getRecord();

    auto dlg = new ActionDialog(this);
    dlg->setRecord(rec);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect(dlg, &ActionDialog::addAction,
            actions_model_, &ActionsModel::addAction);
    dlg->exec();
    actions_model_->select();
}

void MainWindow::on_actionEdit_Action_triggered()
{
    auto current = ui->actionsView->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return;
    }

    auto dlg = new ActionDialog(this);
    dlg->setModel(actions_model_, current);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    if (dlg->exec() == QDialog::Accepted) {
        actions_model_->openNextActions();
    }
}

void MainWindow::on_actionDelete_Action_triggered()
{
    auto selected = ui->actionsView->selectionModel()->selection().indexes();

    if (selected.isEmpty()) {
        return;
    }

    if (!confirmDelete("Action")) {
        return;
    }

    ui->actionsView->setCurrentIndex({});
    actions_model_->removeActions(selected);
}

void MainWindow::on_actionAction_Done_triggered()
{
    auto current = ui->actionsView->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return;
    }

    actions_model_->setCompleted(current);
}

void MainWindow::on_actionExecute_Action_triggered()
{
    auto selected = ui->actionsView->currentIndex();

    if (!selected.isValid()) {
        return;
    }

    // Find contact.
    int person = actions_model_->data(
                actions_model_->index(selected.row(), actions_model_->property("person_col").toInt(), {}),
                Qt::DisplayRole).toInt();

    if (!person) {
        person = actions_model_->data(
                    actions_model_->index(selected.row(), actions_model_->property("contact_col").toInt(), {}),
                    Qt::DisplayRole).toInt();
    }

    if (!person) {
        qWarning() << "Cannot find contact or person for action!";
        return;
    }

    const auto type = actions_model_->data(
                actions_model_->index(selected.row(), actions_model_->property("type_col").toInt(), {}),
                Qt::DisplayRole).toInt();

    if (type != static_cast<int>(ActionType::CHANNEL)) {
        QMessageBox::warning(this,
                             "Cannot Execute",
                             "Can only exceute actions that are of type Channel");

        return;
    }

    const auto channel_type = actions_model_->data(
                actions_model_->index(selected.row(), actions_model_->property("channel_type_col").toInt(), {}),
                Qt::DisplayRole).toInt();

    // See if the requested contact-person have any such channels

    QSqlQuery query(QStringLiteral(
        "select value from channel where contact = %1 and type = %2 order by value")
                    .arg(person)
                    .arg(channel_type));

    QList<QString> values;
    while(query.next()) {
        values.push_back(query.value(0).toString());
    }

    if (values.isEmpty()) {
        QMessageBox::warning(this,
                             "Cannot Execute",
                             QStringLiteral(
                                 "The relevant contact does not have any %1 defined.")
                             .arg(GetChannelTypeName(channel_type)));

        return;
    }


    auto dlg = new ActionExecuteDialog(values, channel_type, this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    if (dlg->exec() == QDialog::Accepted) {
        openChannel(ToChannelType(channel_type), dlg->value);

        if (QMessageBox::question(this,
                                  "Action",
                                  "Do you want to mark this action as done?")
                == QMessageBox::Yes) {
            actions_model_->setCompleted(selected);
        }
    }
}

void MainWindow::on_actionMove_Action_Up_triggered()
{
    auto current = ui->actionsView->selectionModel()->currentIndex();
    actions_model_->moveUp(current);
}

void MainWindow::on_actionMove_Action_Down_triggered()
{
    auto current = ui->actionsView->selectionModel()->currentIndex();
    actions_model_->moveDown(current);
}

void MainWindow::on_actionAdd_Document_triggered()
{
    const auto current = ui->contactsList->currentIndex();

    if (!current.isValid()) {
        return;
    }

    const auto contact_id = contacts_model_->getContactId(current);
    auto rec = documents_model_->getRecord(contact_id, Document::Type::NOTE,
                                           Document::Class::NOTE,
                                           Document::Direction::INTERNAL,
                                           Document::Entity::CONTACT);

    auto dlg = new DocumentDialog(rec, 0, this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect(dlg, &DocumentDialog::addDocument,
            documents_model_, &DocumentsModel::addDocument);
    dlg->exec();
}

void MainWindow::on_actionEdit_Document_triggered()
{
    auto current = ui->documentsView->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return;
    }

    auto dlg = new DocumentDialog(documents_model_->record(current.row()), current.row(), this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect(dlg, &DocumentDialog::updateDocument,
            documents_model_, &DocumentsModel::updateDocument);
    dlg->exec();
}

void MainWindow::on_actionDelete_Document_triggered()
{
    auto selected = ui->documentsView->selectionModel()->selection().indexes();

    if (selected.isEmpty()) {
        return;
    }

    if (!confirmDelete("Document")) {
        return;
    }

    ui->documentsView->setCurrentIndex({});
    documents_model_->removeDocuments(selected);
}

void MainWindow::on_actionOpen_Document_triggered()
{
    auto current = ui->documentsView->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return;
    }

    const auto type = Document::toType(
                documents_model_->data(
                    documents_model_->index(current.row(),
                                            documents_model_->property("type_col").toInt()),
                    Qt::DisplayRole).toInt());
    const auto what = documents_model_->data(
                documents_model_->index(current.row(),
                                        documents_model_->property("location_col").toInt()),
                Qt::DisplayRole).toString();

    if (type == Document::Type::NOTE) {
        on_actionEdit_Document_triggered();
    } else {
        Document::open(type, what);
    }
}
