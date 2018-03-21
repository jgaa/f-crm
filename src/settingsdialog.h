#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QSettings& settings, QWidget *parent = 0);
    ~SettingsDialog();

signals:
    void logSettingsChanged();

private:
    Ui::SettingsDialog *ui;
    QSettings& settings_;

    // QDialog interface
public slots:
    void accept() override;

private slots:
    void selectDbFile();
};

#endif // SETTINGSDIALOG_H
