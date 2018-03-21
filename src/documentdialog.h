#ifndef DOCUMENTDIALOG_H
#define DOCUMENTDIALOG_H

#include <QSqlRecord>
#include <QDialog>
#include <QDataWidgetMapper>
#include <QComboBox>

#include "document.h"
#include "documentsmodel.h"

namespace Ui {
class DocumentDialog;
}

class DocumentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DocumentDialog(const QSqlRecord& rec, int row = 0, QWidget *parent = 0);
    ~DocumentDialog();

signals:
    void addDocument(const QSqlRecord& rec);
signals:
    void updateDocument(const int row, const QSqlRecord& rec);

private slots:
    void onEntityCurrentIndexChanged(int index);
    void onTypeCurrentIndexChanged(int index);
    void onLocationBtnClicked(bool checked);
    void onOpenBtnClicked(bool checked);

private:
    void syncEntity();
    void syncEntityToRec();
    void syncType();

    Ui::DocumentDialog *ui;
    QSqlRecord rec_;
    const int row_;

    // QDialog interface
    void fecthPersons();
    QVariant getValue(const char *colName) const;
    void fetchContacts(int contactId, QComboBox *combo);
    void fetchPersons(int contactId, QComboBox *combo);
    void fetchIntents(int contactId, QComboBox *combo);
    void fetchActions(int contactId, QComboBox *combo);
public slots:
    void accept() override;
    void reject() override;
};

#endif // DOCUMENTDIALOG_H
