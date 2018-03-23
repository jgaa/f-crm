#include "src/tableviewwithdrop.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>

TableViewWithDrop::TableViewWithDrop(QWidget *parent)
    : QTableView(parent)
{
    setAcceptDrops(true);
}

void TableViewWithDrop::setDocumentsModel(DocumentsModel *model)
{
    document_model_ = model;
}

void TableViewWithDrop::setContactId(const int id)
{
    contact_id_ = id;
}

void TableViewWithDrop::setEntity(Document::Entity entity, QSqlTableModel *model, const int id)
{
    entity_ = entity;
    entity_id_ = id;
    entity_model_ = model;
}

bool TableViewWithDrop::canDoDrop() const
{
    return enabled_;
}


void TableViewWithDrop::dragEnterEvent(QDragEnterEvent *event)
{
    setBackgroundRole(QPalette::Highlight);

    if (canDoDrop()) {
        event->acceptProposedAction();
    }
}

void TableViewWithDrop::dragMoveEvent(QDragMoveEvent *event)
{
    if (canDoDrop()) {
        event->acceptProposedAction();
    }
}

void TableViewWithDrop::dragLeaveEvent(QDragLeaveEvent *event)
{
    if (canDoDrop()) {
        event->accept();
    }
}

void TableViewWithDrop::dropEvent(QDropEvent *event)
{
    if (canDoDrop()) {
        const QMimeData *mime = event->mimeData();

        const auto formats = mime->formats();
        qDebug() << formats;
        if (mime->hasUrls()) {
            const auto urls = mime->urls();
            qDebug() << urls;
        }
        if (mime->hasHtml()) {
            const auto html = mime->html();
            qDebug() << html;
        }
        if (mime->hasText()) {
            const auto text = mime->text();
            qDebug() << text;
        }

        if (mime->hasUrls() && !mime->urls().isEmpty()) {
            const auto url = mime->urls().first();
            //const auto type = Document::deduceType(url);

            // Map entity id to the row uder the cursor
            if (entity_model_) {
                auto droppoint =  event->pos();
                const auto row = this->rowAt(droppoint.ry());
                if (row >= 0) {
                    event->acceptProposedAction();
                    addDocument(row, url);
                    return;
                }
            } else {
                // Add to the pane, disregard what item that was under the cursor
                event->acceptProposedAction();
                addDocument(-1, url);
                return;
            }
        }

        event->setAccepted(false);
    }
}

void TableViewWithDrop::addDocument(const int row, const QUrl& url)
{
    int id = -1;
    if (row >= 0) {

        Q_ASSERT(entity_model_);
        id = entity_model_->data(entity_model_->index(row, entity_model_->property("id_col").toInt(), {}), Qt::DisplayRole).toInt();
        if (id <= 0) {
            QMessageBox::warning(this, "Failed to get the ID for the entity",
                                 "You must drop on an item in the list");
            qWarning() << "Failed to get the ID for the entity";
            return;
        }

    } else {
        id = entity_id_;
    }

    if (id < 0) {
        QMessageBox::warning(this, "No entity id to drop to",
                             "You must drop on an item in the list");
        qWarning() << "No entity id to drop to";
        return;
    }

    const auto type = Document::deduceType(url);

    int contact = contact_id_, person = 0, intent = 0, action = 0;
    if (entity_ == Document::Entity::CONTACT) {
        contact = id;
    } else if (entity_ == Document::Entity::PERSON) {
        person = id;
    } else if (entity_ == Document::Entity::INTENT) {
        intent = id;
    } else if (entity_ == Document::Entity::ACTION) {
        action = id;
    }

    auto rec = document_model_->getRecord(contact,
                                           type,
                                           Document::Class::NOTE,
                                           Document::Direction::INTERNAL,
                                           entity_, person, intent, action);
    rec.setValue("location", url.toString());

    if (type == Document::Type::FILE) {
        const auto path = url.path();
        const QFileInfo fi{path};
        const auto name = fi.fileName();
        rec.setValue("name", name);
        rec.setValue("file_date", fi.lastModified());
    }

    auto dlg = new DocumentDialog(rec, 0, this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect(dlg, &DocumentDialog::addDocument,
            document_model_, &DocumentsModel::addDocument);
    dlg->exec();

}
