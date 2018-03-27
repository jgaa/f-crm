#include "src/settingsdialog.h"
#include "ui_settingsdialog.h"
#include "logging.h"

#include <QFileDialog>

SettingsDialog::SettingsDialog(QSettings& settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    settings_{settings}
{
    ui->setupUi(this);

    ui->dbPathEdit->setText(settings_.value("dbpath", "").toString());
    ui->saveWindowState->setCheckState(
                settings_.value("restore-window-state", true).toBool()
                ? Qt::Checked : Qt::Unchecked);

    ui->logPathEdit->setText(settings_.value("log-path", "f-crm.log").toString());
    ui->emailApp->setText(settings_.value("mailapp", "").toString());

    ui->enableLoggingCheck->setCheckState(
                settings_.value("log-enabled", false).toBool()
                ? Qt::Checked : Qt::Unchecked);
    ui->logAppendCheck->setCheckState(
                settings.value("log-append", false).toBool()
                ? Qt::Checked : Qt::Unchecked);
    ui->logPathEdit->setText(settings_.value("log-path", "whid.log").toString());

    ui->tabCtl->setCurrentIndex(0);

    connect(ui->dbSelectPathBtn, SIGNAL(clicked()), this, SLOT(selectDbFile()));
    connect(this, SIGNAL(logSettingsChanged()), Logging::instance(), SLOT(changed()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


void SettingsDialog::accept()
{
    settings_.setValue("dbpath", ui->dbPathEdit->text());
    settings_.setValue("mailapp", ui->emailApp->text());

    settings_.setValue("restore-window-state",
                      ui->saveWindowState->checkState() == Qt::Checked);

    bool log_changed = false;

    if (settings_.value("log-enabled",false)
            != (ui->enableLoggingCheck->checkState() == Qt::Checked)) {
        log_changed = true;
    }

    settings_.setValue("log-enabled",
                      ui->enableLoggingCheck->checkState() == Qt::Checked);


    if (settings_.value("log-path") != ui->logPathEdit->text()) {
        log_changed = true;
    }

    settings_.setValue("log-path", ui->logPathEdit->text());

    settings_.setValue("log-append",
                      ui->logAppendCheck->checkState() == Qt::Checked);


    if (log_changed) {
        emit logSettingsChanged();
    }

    QDialog::accept();
}

void SettingsDialog::selectDbFile()
{
    auto path = QFileDialog::getSaveFileName(this,
                                             "Select Datatabase",
                                             ui->dbPathEdit->text(),
                                             "SQLite Files (*.db)",
                                             Q_NULLPTR,
                                             QFileDialog::DontConfirmOverwrite);

    if (!path.isNull() && !path.isEmpty()) {
        ui->dbPathEdit->setText(path);
    }
}
