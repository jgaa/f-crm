#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QDataWidgetMapper>
#include <QItemSelection>
#include <QMainWindow>
#include <QSettings>
#include <QTableView>

#include "contact.h"
#include "channel.h"
#include "contactsmodel.h"
#include "channelsmodel.h"
#include "database.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum class AppMode {
        PANEL,
        CONTACTS
    };

    enum class PersonTab {
        PANEL,
        CONTACT,
        INTENTS,
        LOG,
        DOCUMENTS
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void initialize();

signals:

private slots:
    void appModeSelectionChanged();
    void onContactFilterChanged(const QString& text);
    void onContactsListRowActivated(const QModelIndex &index);
    void onContactsListSelectionChanged(
            const QItemSelection &selected, const QItemSelection &deselected);
    void onContactsListCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void onContactsModelReset();
    void onSyncronizeContactsBindings();
    void onSyncronizePersonBindings();
    void onValidateContactActions();
    void onContactContextMenuRequested(const QPoint &pos);

    void onChannelContextMenuRequested(const QPoint &pos);
    void onChannelListRowActivated(const QModelIndex &index);
    void onChannelsModelReset();
    void onValidateChannelActions();

    void onPersonsContextMenuRequested(const QPoint &pos);
    void onPersonsListRowActivated(const QModelIndex &index);
    void onPersonsModelReset();
    void onPersonsClicked(const QModelIndex &index);
    void onValidatePersonsActions();

    void onContactTabChanged(int ix);

    void on_action_Quit_triggered();
    void on_actionAdd_Contact_triggered();

    void on_actionDelete_Contact_triggered();

    void on_actionAdd_Channel_triggered();

    void on_actionDelete_Channel_triggered();

    void on_actionEdit_Channel_triggered();

    void on_actionVerify_Channel_triggered();

    void on_actionCopy_Channel_to_Clipboard_triggered();

    void on_actionOpen_Channel_triggered();

    void on_actionAdd_Person_triggered();

    void on_actionAdd_Company_triggered();

    void on_actionEdit_Person_triggered();

    void on_actionDelete_Person_triggered();

private:
    QString getChannelValue() const;
    ChannelType getChannelType() const;
    void createContact(ContactType type);


    // Get the current person (either a company/contact - upper list) or a
    // person in a company (lower list), depending on the current context.

    // Return true if the current person is part of the selected company (lower list)
    bool isCurrentPersonInCompany() const;
    // Return the view for the current person
    QTableView *getCurrentPersonView() const;
    // Return the current selection index for the current person
    QModelIndex getCurrentPersonIndex() const;
    // return the model for the current person
    ContactsModel *getCurrentPersonModel() const;
    // returns 0 if no person is current and selected;
    int getCurrentPersonId() const;

    Ui::MainWindow *ui;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
    AppMode app_mode_ = AppMode::PANEL;
    QSettings settings_;
    std::unique_ptr<Database> db_ = {};
    ContactsModel *contacts_model_ = {};
    ContactsModel *persons_model_ = {}; // contact (persons) at a contact (company)
    ChannelsModel *channels_model_ = {};
    std::unique_ptr<QDataWidgetMapper> contacts_mapper_;
    std::unique_ptr<QDataWidgetMapper> persons_mapper_;
    int last_person_clicked {-1};
};

#endif // MAINWINDOW_H
