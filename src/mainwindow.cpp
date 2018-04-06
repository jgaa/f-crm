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
#include "settingsdialog.h"
#include "logging.h"
#include "favoritesdialog.h"
#include "aboutdialog.h"
#include "strategy.h"

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

#ifdef QT_DEBUG
    this->setWindowTitle(this->windowTitle() + " Debug");
#endif

    db_ = std::make_unique<Database>(nullptr);

    log_model_ = new JournalModel(settings_, this, {});
    log_px_model_ = new JournalProxyModel(log_model_, this);
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
    contact_upcoming_model_ = new UpcomingModel(settings_, this,
                                                UpcomingModel::Mode::CONTACT_UPCOMING);
    today_model_= new UpcomingModel(settings_, this,
                                    UpcomingModel::Mode::TODAY);
    upcoming_model_ = new UpcomingModel(settings_, this,
                                        UpcomingModel::Mode::UPCOMING);

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
                channels_model_->fieldIndex("value"), QHeaderView::Stretch);
    ui->contactChannels->horizontalHeader()->moveSection(channels_model_->fieldIndex("name"), 0);
    for(int i = 0; i < channels_model_->columnCount(); ++i) {
        const bool show = (i == channels_model_->fieldIndex("value"))
                || (i == channels_model_->fieldIndex("name"))
                || (i == channels_model_->fieldIndex("verified"));
        ui->contactChannels->setColumnHidden(i, !show);
    }

    ui->contactPeople->setModel(person_px_model);
    ui->contactPeople->setDocumentsModel(documents_model_);
    ui->contactPeople->horizontalHeader()->setSectionResizeMode(
                persons_model_->property("name_col").toInt(), QHeaderView::Stretch);
    for(int i = 0; i < persons_model_->columnCount(); ++i) {
        const bool show =
                (i == persons_model_->property("name_col").toInt())
                /*|| (i == persons_model_->property("status_col").toInt())*/;
        ui->contactPeople->setColumnHidden(i, !show);
    }

    ui->intentsView->setModel(intents_px_model_);
    ui->intentsView->setDocumentsModel(documents_model_);

    ui->intentsView->horizontalHeader()->setSectionResizeMode(
                intents_model_->fieldIndex("abstract"), QHeaderView::Stretch);
    ui->intentsView->horizontalHeader()->moveSection(intents_model_->fieldIndex("created_date"), 0);
    for(int i = 0; i < intents_model_->columnCount(); ++i) {
        const bool show = (i == intents_model_->fieldIndex("abstract"))
                || (i == intents_model_->fieldIndex("created_date"))
                || (i == intents_model_->fieldIndex("state"));
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

    ui->contactUpcomingTable->setModel(contact_upcoming_model_);
    ui->contactUpcomingTable->horizontalHeader()->setSectionResizeMode(
                contact_upcoming_model_->H_NAME, QHeaderView::Stretch);
    ui->contactUpcomingTable->setColumnHidden(contact_upcoming_model_->H_ID, true);
    ui->contactUpcomingTable->setColumnHidden(contact_upcoming_model_->H_CONTACT_ID, true);
    ui->contactUpcomingTable->setColumnHidden(contact_upcoming_model_->H_CONTACT_NAME, true);
    ui->contactUpcomingTable->setColumnHidden(contact_upcoming_model_->H_CONTACT_STATUS, true);
    ui->contactUpcomingTable->setColumnHidden(contact_upcoming_model_->H_STATE, true);
    ui->contactUpcomingTable->setColumnHidden(contact_upcoming_model_->H_INTENT_ID, true);
    ui->contactUpcomingTable->setColumnHidden(contact_upcoming_model_->H_PERSON_ID, true);
    ui->contactUpcomingTable->setColumnHidden(contact_upcoming_model_->H_DESIRED_OUTCOME, true);

    ui->actionsToday->setModel(today_model_);
    ui->actionsToday->horizontalHeader()->setSectionResizeMode(
                today_model_->H_NAME, QHeaderView::Stretch);
    ui->actionsToday->setColumnHidden(today_model_->H_ID, true);
    ui->actionsToday->setColumnHidden(today_model_->H_CONTACT_ID, true);
    ui->actionsToday->setColumnHidden(contact_upcoming_model_->H_CONTACT_STATUS, true);
    ui->actionsToday->setColumnHidden(today_model_->H_STATE, true);
    ui->actionsToday->setColumnHidden(today_model_->H_INTENT_ID, true);
    ui->actionsToday->setColumnHidden(today_model_->H_PERSON_ID, true);
    ui->actionsToday->setColumnHidden(today_model_->H_DESIRED_OUTCOME, true);

    ui->actionsUpcoming->setModel(upcoming_model_);
    ui->actionsUpcoming->horizontalHeader()->setSectionResizeMode(
                upcoming_model_->H_NAME, QHeaderView::Stretch);
    ui->actionsUpcoming->setColumnHidden(upcoming_model_->H_ID, true);
    ui->actionsUpcoming->setColumnHidden(upcoming_model_->H_CONTACT_ID, true);
    ui->actionsUpcoming->setColumnHidden(contact_upcoming_model_->H_CONTACT_STATUS, true);
    ui->actionsUpcoming->setColumnHidden(upcoming_model_->H_STATE, true);
    ui->actionsUpcoming->setColumnHidden(upcoming_model_->H_INTENT_ID, true);
    ui->actionsUpcoming->setColumnHidden(upcoming_model_->H_PERSON_ID, true);
    ui->actionsUpcoming->setColumnHidden(upcoming_model_->H_DESIRED_OUTCOME, true);


    ui->contactTab->setCurrentIndex(0);


    connect(ui->contactFilter, &QLineEdit::textChanged, this, &MainWindow::onContactFilterChanged);
    connect(ui->contactsList, &QTableView::activated, this, &MainWindow::onContactsListRowActivated);
    connect(ui->contactsList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onContactsListSelectionChanged);
    connect(ui->contactsList->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onContactsListCurrentChanged);
    connect(contacts_model_, &ContactsModel::modelReset, this, &MainWindow::onContactsModelReset);
    connect(contacts_model_, &ContactsModel::dataChanged, this, &MainWindow::onContactsDataChanged);
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
    connect(intents_model_, &IntentsModel::dataChanged,
            this, &MainWindow::onIntentsDataChanged, Qt::QueuedConnection);

    connect(ui->actionsView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onActionsRowActivated);
    connect(ui->actionsView, &QTableView::customContextMenuRequested,
            this, &MainWindow::onActionsContextMenuRequested);
    connect(actions_model_, &ActionsModel::modelReset,
            this, &MainWindow::onActionsModelReset);
    connect(actions_model_, &ActionsModel::dataChanged,
            this, &MainWindow::onActionsDataChanged, Qt::QueuedConnection);
    connect(actions_model_, &ActionsModel::rowsInserted,
            this, &MainWindow::onActionsrowsInserted);


    connect(ui->documentsView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onDocumentsRowActivated);
    connect(ui->documentsView, &QTableView::customContextMenuRequested,
            this, &MainWindow::onDocumentsContextMenuRequested);
    connect(documents_model_, &DocumentsModel::modelReset,
            this, &MainWindow::onDocumentsModelReset);
    connect(documents_model_, &DocumentsModel::dataChanged,
            this, &MainWindow::onDocumentsDataChanged);


    connect(ui->contactTab, &QTabWidget::currentChanged, this, &MainWindow::onContactTabChanged);

    connect(Logging::instance(), &Logging::message, this, &MainWindow::showMessage, Qt::QueuedConnection);
    connect(ui->clearFilter, &QToolButton::clicked, this, &MainWindow::clearFilter);

    onSyncronizeContactsBindings();
}

void MainWindow::showMessage(const QString &label, const QString &text)
{
    QMessageBox::warning(this, label, text);
}

void MainWindow::setFilter(QString value)
{
    ui->contactFilter->setText(value);
}

void MainWindow::clearFilter(bool)
{
    ui->contactFilter->setText("");
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

    if (app_mode_ == AppMode::PANEL) {
        upcoming_model_->select();
        today_model_->select();
    }
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

    if (current.isValid()) {

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

        syncContactData(contacts_model_, current.row());

        intents_model_->setContact(contact_id);
        actions_model_->setContact(contact_id);
        log_model_->setContact(contact_id);
        documents_model_->setContact(contact_id);
        contact_upcoming_model_->setContact(contact_id);

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
        setupMapper(current.row());

    } else {
        syncContactData();
        clearMapper();

        channels_model_->setContact(-1);
        ui->contactPeople->setEnabled(false);

        persons_model_->setParent(-1);
        persons_model_->select();

        ui->personWhoIcon->setPixmap({});
        ui->personWhoName->setText({});

        actions_model_->setContact(-1);
        intents_model_->setContact(-1);
        log_model_->setContact(-1);
        documents_model_->setContact(-1);
        contact_upcoming_model_->setContact();

        ui->documentsView->setContactId(-1);
        ui->documentsView->setDocumentDropEnabled(false);
        ui->contactPeople->setDocumentDropEnabled(false);
        ui->intentsView->setDocumentDropEnabled(false);
        ui->actionsView->setDocumentDropEnabled(true);
    }

    last_person_clicked = -1;

    onSyncronizePersonBindings();
    onValidateContactActions();
    onValidatePersonsActions();
}

void MainWindow::onSyncronizePersonBindings()
{
    const auto current = ui->contactsList->currentIndex();

    if (current.isValid()) {
        const auto people = ui->contactPeople->currentIndex();
        if (people.isValid()) {
            const auto contact_id = persons_model_->getContactId(people);
            channels_model_->setContact(contact_id);
            syncPersonData(persons_model_, people.row());
            setupMapper(people.row());
        } else {
            const auto contact_id = contacts_model_->getContactId(current);
            channels_model_->setContact(contact_id);
            syncPersonData(contacts_model_, current.row());
            setupMapper(current.row());
        }
    } else {
        syncPersonData();
        clearMapper();
    }
}

void MainWindow::onValidateContactActions()
{
    const auto current = ui->contactsList->currentIndex();
    const bool enabled = app_mode_ == AppMode::CONTACTS;
    const bool enable_modifications = enabled && current.isValid();
    const bool enable_add_person = enabled && current.isValid()
            && ToContactType(
                contacts_model_->data(
                    contacts_model_->index(current.row(),
                                           contacts_model_->property("type_col").toInt(), {}),
                    Qt::DisplayRole).toInt()) == ContactType::CORPORATION;

    ui->actionAdd_Contact->setEnabled(enabled);
    ui->actionAdd_Company->setEnabled(enabled);
    ui->actionAdd_Person->setEnabled(enable_add_person);

    ui->actionEdit_Contact->setEnabled(enable_modifications);
    ui->actionDelete_Contact->setEnabled(enable_modifications);
}

void MainWindow::onContactContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu;

    menu->addAction(ui->actionAdd_Company);
    menu->addAction(ui->actionAdd_Contact);
    menu->addAction(ui->actionEdit_Contact);
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
    const auto current = ui->contactPeople->currentIndex().isValid()
            ? ui->contactPeople->currentIndex()
            : ui->contactsList->currentIndex();
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

    //ui->actionAdd_Person->setEnabled(enable);

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

void MainWindow::onIntentsDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)
{
    actions_model_->updateState();
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
    contact_upcoming_model_->select();
    intents_model_->updateState();
}

void MainWindow::onActionsrowsInserted(const QModelIndex &, int , int )
{
    intents_model_->updateState();
}

void MainWindow::onActionsModelReset()
{
    onValidateActionActions();
    contact_upcoming_model_->select();
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

    if (mapper_){
        mapper_->submit();
    }

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

void MainWindow::setupMapper(const int row)
{
    if (disable_mapper_) {
        return;
    }

    if (mapper_) {
        mapper_->submit();

        if (mapper_is_ == (isCurrentPersonInCompany() ? Mapper::PERSON : Mapper::CONTACT)) {
            mapper_->setCurrentIndex(row);
            return; // No need to do anything more
        }
    }

    mapper_ = make_unique<QDataWidgetMapper>();
    ContactsModel *model = {};

    if (isCurrentPersonInCompany()) {
        mapper_is_ = Mapper::PERSON;
        model = persons_model_;
    } else {
        mapper_is_ = Mapper::CONTACT;
        model = contacts_model_;
    }

    mapper_->setModel(model);

    mapper_->addMapping(ui->contactAddress, model->fieldIndex("address1"));
    mapper_->addMapping(ui->contactAddress2, model->fieldIndex("address2"));
    mapper_->addMapping(ui->contactCity, model->fieldIndex("city"));
    mapper_->addMapping(ui->contactPostCode, model->fieldIndex("postcode"));
    mapper_->addMapping(ui->contactCountry, model->fieldIndex("country"));
    mapper_->addMapping(ui->personNotes, model->fieldIndex("notes"));

    mapper_->setCurrentIndex(row);
}

void MainWindow::clearMapper()
{
    if (mapper_) {
        mapper_->submit();
        mapper_->clearMapping();
        mapper_.reset();
        mapper_is_ = Mapper::NOONE;
    }

    ui->contactAddress->clear();
    ui->contactAddress2->clear();
    ui->contactCity->clear();
    ui->contactPostCode->clear();
    ui->contactCountry->clear();
    ui->personNotes->clear();
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

    disable_mapper_ = true;
    clearMapper();

    const auto contact_id = contacts_model_->getContactId(current);

    auto rec = persons_model_->record();
    rec.setValue("contact", contact_id);
    rec.setValue("type", static_cast<int>(ContactType::INDIVID));

    persons_model_->addPerson(rec);
    auto dlg = new PersonDialog(*persons_model_, true, 0, this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->exec();
    disable_mapper_ = false;
    const auto ix = person_px_model->index(0,persons_model_->fieldIndex("name"));

    // Had some problems selecting the new person... Let's be really explicit here.
    ui->contactPeople->scrollTo(ix);
    ui->contactPeople->setFocus();
    ui->contactPeople->selectionModel()->select(ix, QItemSelectionModel::Clear);
    ui->contactPeople->selectionModel()->select(ix, QItemSelectionModel::Select);
    ui->contactPeople->setCurrentIndex(ix);
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
    disable_mapper_ = true;
    clearMapper();

    contacts_model_->createContact(type);

    auto dlg = new PersonDialog(*contacts_model_, false, 0, this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->exec();
    disable_mapper_ = false;
    const auto ix = contact_px_model->index(0, contacts_model_->fieldIndex("name"));
    ui->contactsList->scrollTo(ix);
    ui->contactsList->setFocus();
    ui->contactsList->selectionModel()->select(ix, QItemSelectionModel::Clear);
    ui->contactsList->selectionModel()->select(ix, QItemSelectionModel::Select);
    ui->contactsList->setCurrentIndex(ix);
}

bool MainWindow::isCurrentPersonInCompany() const
{
    return ui->contactPeople->currentIndex().isValid();
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

void MainWindow::syncPersonData(ContactsModel *model, const int row)
{
    ui->personWhoIcon->setPixmap(contactData(model, "type", row, Qt::DecorationRole).value<QIcon>().pixmap(24,24));
    ui->personWhoName->setText(contactData(model, "name", row).toString());
}

void MainWindow::syncContactData(ContactsModel *model, const int row)
{
    ui->contactWhoIcon->setPixmap(contactData(model, "type", row, Qt::DecorationRole).value<QIcon>().pixmap(24,24));
    ui->contactWhoName->setText(contactData(model, "name", row).toString());
    ui->contactStatusIcon->setPixmap(contactData(model, "status", row,
                                                 Qt::DecorationRole).value<QIcon>().pixmap(24,24));
    ui->contactFavoriteIcon->setPixmap(contactData(model, "favourite", row, Qt::DecorationRole).value<QIcon>().pixmap(24,24));
    ui->contactStarsIcon->setPixmap(contactData(model, "stars", row, Qt::DecorationRole).value<QIcon>().pixmap(80,16));

    const bool visible = model != nullptr;
    ui->contactFavoriteIcon->setVisible(visible);
    ui->contactStarsIcon->setVisible(visible);
    ui->contactWhoIcon->setVisible(visible);
}

QVariant MainWindow::contactData(ContactsModel *model, const char *col, const int row, const int role)
{
    if (model) {
        Q_ASSERT(col);
        Q_ASSERT(row >= 0);
        return model->data(model->index(row, model->fieldIndex(col), {}), role);
    }
    return {};
}


void MainWindow::on_actionEdit_Person_triggered()
{
    auto current = ui->contactPeople->selectionModel()->currentIndex();
    if (!current.isValid()) {
        return;
    }

    disable_mapper_ = true;
    clearMapper();
    auto dlg = new PersonDialog(*persons_model_, true, current.row(),  this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->exec();
    disable_mapper_ = false;
    setupMapper(current.row());
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

    auto dlg = new ActionDialog(rec.value("person").toInt(), this);
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

    auto dlg = new ActionDialog(actions_model_->contact(), this);
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

void MainWindow::on_actionSettings_triggered()
{
    auto dlg = new SettingsDialog{settings_, this};
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->exec();
}

void MainWindow::on_actionEdit_Contact_triggered()
{
    auto current = ui->contactsList->currentIndex();
    if (!current.isValid()) {
        return;
    }

    disable_mapper_ = true;
    clearMapper();
    auto dlg = new PersonDialog(*contacts_model_, false, current.row(),  this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->exec();
    disable_mapper_ = false;
    setupMapper(current.row());
}


void MainWindow::onContactsDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int>)
{
    auto current = ui->contactsList->currentIndex();
    if (current.isValid()) {
        syncContactData(contacts_model_, current.row());
    } else {
        syncContactData();
    }
    onSyncronizePersonBindings();
}

void MainWindow::on_actionRateContact_triggered()
{
    auto current = ui->contactsList->currentIndex();
    if (!current.isValid()) {
        return;
    }

    const auto stars = contactData(contacts_model_, "stars", current.row()).toInt();

    auto dlg = new FavoritesDialog(current.row(), stars, this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect(dlg, &FavoritesDialog::setStars,
            contacts_model_, &ContactsModel::setStars);
    dlg->exec();
}

void MainWindow::on_action_About_triggered()
{
    auto dlg = new AboutDialog(this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    dlg->exec();
}
