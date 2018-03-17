#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QDataWidgetMapper>
#include <QItemSelection>
#include <QMainWindow>
#include <QSettings>

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
    void onValidateContactActions();
    void onContactContextMenuRequested(const QPoint &pos);

    void onChannelContextMenuRequested(const QPoint &pos);
    void onChannelListRowActivated(const QModelIndex &index);
    void onChannelsModelReset();
    void onValidateChannelActions();

    void onPersonsContextMenuRequested(const QPoint &pos);
    void onPersonsListRowActivated(const QModelIndex &index);
    void onPersonsModelReset();
    void onValidatePersonsActions();

    void on_action_Quit_triggered();
    void on_actionAdd_Contact_triggered();

    void on_actionDelete_Contact_triggered();

    void on_actionAdd_Channel_triggered();

    void on_actionDelete_Channel_triggered();

    void on_actionEdit_Channel_triggered();

    void on_actionVerify_Channel_triggered();

    void on_actionCopy_Channel_to_Clipboard_triggered();

    void on_actionOpen_Channel_triggered();

private:
    QString getChannelValue() const;
    ChannelType getChannelType() const;

    Ui::MainWindow *ui;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
    QSettings settings_;
    std::unique_ptr<Database> db_ = {};
    ContactsModel *contacts_model_ = {};
    ContactsModel *persons_model_ = {}; // contact (persons) at a contact (company)
    ChannelsModel *channels_model_ = {};
    std::unique_ptr<QDataWidgetMapper> contacts_mapper_;
};

#endif // MAINWINDOW_H
