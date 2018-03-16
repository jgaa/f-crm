#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>

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
//    QIcon appicon(":res/icons/f-crm.svg");
//    setWindowIcon(appicon);

    if (settings_.value("restore-window-state", true).toBool()) {
        restoreGeometry(settings_.value("windowGeometry").toByteArray());
        restoreState(settings_.value("windowState").toByteArray());
    }

    db_ = std::make_unique<Database>(nullptr);

    contacts_model_ = new ContactsModel(settings_, this, {});
    ui->contactsList->setModel(contacts_model_);
    ui->contactsList->setModelColumn(contacts_model_->getNameCol());
    //contacts_model_->select()

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
}

void MainWindow::appModeSelectionChanged()
{
    int selection = ui->appModeList->currentRow();
    ui->stackedWidget->setCurrentIndex(selection);

    const bool is_contact = selection == 1;
    ui->actionAdd_Contact->setEnabled(is_contact);
    ui->menuContact->setEnabled(is_contact);
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
    const auto ix = contacts_model_->createContact();
    if (ix.isValid()) {
        ui->contactsList->setFocus();
        ui->contactsList->setCurrentIndex(ix);
        ui->contactsList->edit(ix);
    }
}
