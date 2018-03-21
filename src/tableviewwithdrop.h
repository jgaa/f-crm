#ifndef TABLEVIEWWITHDROP_H
#define TABLEVIEWWITHDROP_H

#include "document.h"
#include "documentsmodel.h"
#include "documentdialog.h"
#include <QTableView>


class TableViewWithDrop : public QTableView
{
public:
    TableViewWithDrop(QWidget *parent = Q_NULLPTR);

    void setDocumentDropEnabled(bool enable) { enabled_ = enable; }
    void setDocumentsModel(DocumentsModel *model);
    void setContactId(const int id);
    void setEntity(Document::Entity entity, QSqlTableModel *model, const int id);
    bool canDoDrop() const;

    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void addDocument(const int row, const QUrl& url);

    bool enabled_ = false;
    DocumentsModel *document_model_ = {};
    QSqlTableModel *entity_model_ = {};
    int contact_id_ = {};
    int entity_id_ = {};
    Document::Entity entity_ = Document::Entity::CONTACT;
};

#endif // TABLEVIEWWITHDROP_H
