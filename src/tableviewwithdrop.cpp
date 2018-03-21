#include "src/tableviewwithdrop.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>

TableViewWithDrop::TableViewWithDrop(QWidget *parent)
    : QTableView(parent)
{

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
    return contact_id_ > 0;
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

            //event->pos();

            // Add to the pane, disregard what item that was under the cursor
            addDocument(-1, url);

            event->acceptProposedAction();

            return;
        }

        event->setAccepted(false);
    }
}

void TableViewWithDrop::addDocument(const int row, const QUrl& url)
{
    int id = -1;
    if (row >= 0) {
        // TODO: Find the id from the row
    } else {
        id = entity_id_;
    }

    if (id < 0) {
        qWarning() << "No entiry id to drop to";
        return;
    }

    const auto type = Document::deduceType(url);

    auto rec = document_model_->getRecord(contact_id_,
                                           type,
                                           Document::Class::NOTE,
                                           Document::Direction::INTERNAL,
                                           entity_);
    rec.setValue("location", url.toString());

    if (type == Document::Type::FILE) {
        const auto path = url.path();
        const QFileInfo fi{path};
        const auto name = fi.fileName();
        rec.setValue("name", name);
        rec.setValue("file_date", fi.birthTime());
    }

    auto dlg = new DocumentDialog(rec, 0, this);
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect(dlg, &DocumentDialog::addDocument,
            document_model_, &DocumentsModel::addDocument);
    dlg->exec();

}
