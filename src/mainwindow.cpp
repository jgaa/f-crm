#include "channeldialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QDebug>
#include <QSqlRecord>
#include <QClipboard>
#include <QDesktopServices>

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

    contacts_model_ = new ContactsModel(settings_, this, {});
    persons_model_ = new ContactsModel(settings_, this, {});
    persons_model_->setParent(-1);
    channels_model_ = new ChannelsModel(settings_, this, {});
    ui->contactsList->setModel(contacts_model_);
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
        const bool show = (i == contacts_model_->property("name_col").toInt());
        ui->contactsList->setColumnHidden(i, !show);
    }

    ui->contactChannels->setModel(channels_model_);

    ui->contactChannels->horizontalHeader()->setSectionResizeMode(
                channels_model_->property("value_col").toInt(), QHeaderView::Stretch);
    for(int i = 0; i < channels_model_->columnCount(); ++i) {
        const bool show = (i == channels_model_->property("value_col").toInt())
                || (i == channels_model_->property("verified_col").toInt());
        ui->contactChannels->setColumnHidden(i, !show);
    }

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
    connect(persons_model_, &ChannelsModel::modelReset,
            this, &MainWindow::onPersonsModelReset);

    onValidateContactActions();
    onValidatePersonsActions();
}

void MainWindow::appModeSelectionChanged()
{
    int selection = ui->appModeList->currentRow();
    ui->stackedWidget->setCurrentIndex(selection);

    const bool is_contact = selection == 1;
    ui->actionAdd_Contact->setEnabled(is_contact);
    ui->menuContact->setEnabled(is_contact);
}

void MainWindow::onContactFilterChanged(const QString &text)
{
    // We can't use filter() - it don't handle special characters.
    contacts_model_->setNameFilter(text);
}

void MainWindow::onContactsListRowActivated(const QModelIndex &ix)
{
    qDebug() << "Activated: " << (ix.isValid() ? ix.row() : -1);}

void MainWindow::onContactsListSelectionChanged(const QItemSelection &selected,
                                                const QItemSelection &deselected)
{
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
            contacts_mapper_->addMapping(ui->contactAddress, contacts_model_->property("address1_col").toInt());
            contacts_mapper_->addMapping(ui->contactAddress2, contacts_model_->property("address2_col").toInt());
            contacts_mapper_->addMapping(ui->contactCity, contacts_model_->property("city_col").toInt());
            contacts_mapper_->addMapping(ui->contactPostCode, contacts_model_->property("postcode_col").toInt());
            contacts_mapper_->addMapping(ui->contactCountry, contacts_model_->property("country_col").toInt());

            contacts_mapper_->setCurrentIndex(current.row());
        } else {
            if (contacts_mapper_->currentIndex() != current.row()) {
                contacts_mapper_->setCurrentIndex(current.row());
            }
        }

        read_only = false;
        const auto contact_id = contacts_model_->getContactId(current);
        channels_model_->setContact(contact_id);
        persons_model_->setParent(contact_id);
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
        persons_model_->setParent(-1);
    }

    ui->contactNotes->setReadOnly(read_only);
    ui->contactAddress->setReadOnly(read_only);
    ui->contactAddress2->setReadOnly(read_only);
    ui->contactCity->setReadOnly(read_only);
    ui->contactPostCode->setReadOnly(read_only);
    ui->contactCountry->setReadOnly(read_only);

    onValidateContactActions();
    onValidatePersonsActions();
}

void MainWindow::onValidateContactActions()
{
    const auto current = ui->contactsList->currentIndex();
    const bool enable = current.isValid();

    ui->actionDelete_Contact->setEnabled(enable);
    ui->actionAdd_Channel->setEnabled(enable);
}

void MainWindow::onContactContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu;

    menu->addAction(ui->actionAdd_Contact);
    menu->addAction(ui->actionDelete_Contact);

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
    onValidateChannelActions();
}

void MainWindow::onChannelsModelReset()
{
    onValidateChannelActions();
}

void MainWindow::onValidateChannelActions()
{
    const auto current = ui->contactsList->currentIndex();
    const bool enable_channel = current.isValid();
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

}

void MainWindow::onPersonsListRowActivated(const QModelIndex &index)
{

}

void MainWindow::onPersonsModelReset()
{

}

void MainWindow::onValidatePersonsActions()
{
    const auto current = ui->contactsList->currentIndex();
    const bool enable = current.isValid();
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

void MainWindow::on_actionAdd_Contact_triggered()
{
    if (!ui->contactFilter->text().isEmpty()) {
        ui->contactFilter->setText("");
    }
    const auto ix = contacts_model_->createContact();
    if (ix.isValid()) {
        ui->contactsList->setFocus();
        ui->contactsList->setCurrentIndex(ix);
        ui->contactsList->edit(ix);
    }
}

void MainWindow::on_actionDelete_Contact_triggered()
{
    auto selected = ui->contactsList->selectionModel()->selection().indexes();

    if (selected.isEmpty()) {
        return;
    }

    ui->contactsList->setCurrentIndex({});

    contacts_model_->removeContacts(selected);
}

void MainWindow::on_actionAdd_Channel_triggered()
{
    auto current = ui->contactsList->selectionModel()->currentIndex();

    if (!current.isValid()) {
        return;
    }

    const auto contact_id = contacts_model_->getContactId(current);

    auto rec = channels_model_->record();

    rec.setValue("contact", contact_id);

    auto dlg = new ChannelDialog(this);
    dlg->setRecord(rec);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect(dlg, &ChannelDialog::addChannel,
            channels_model_, &ChannelsModel::addChannel);
    dlg->exec();
}

void MainWindow::on_actionDelete_Channel_triggered()
{
    auto selected = ui->contactChannels->selectionModel()->selection().indexes();

    if (selected.isEmpty()) {
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
    auto value = getChannelValue();
    if (!value.isEmpty()) {

        switch(getChannelType()) {
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
