#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QMainWindow>
#include <QSettings>

#include "contactsmodel.h"
#include "database.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void initialize();

signals:

private slots:
    void appModeSelectionChanged();

    void on_action_Quit_triggered();

    void on_actionAdd_Contact_triggered();

private:

    Ui::MainWindow *ui;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
    QSettings settings_;
    std::unique_ptr<Database> db_ = {};
    ContactsModel *contacts_model_ = {};
};

#endif // MAINWINDOW_H
