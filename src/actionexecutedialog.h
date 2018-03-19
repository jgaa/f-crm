#ifndef ACTIONEXECUTEDIALOG_H
#define ACTIONEXECUTEDIALOG_H

#include <QDialog>

#include "channel.h"

namespace Ui {
class ActionExecuteDialog;
}

class ActionExecuteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ActionExecuteDialog(const QList<QString>& data,
                                 const int type,
                                 QWidget *parent = 0);
    ~ActionExecuteDialog();

    QString value;

private:
    Ui::ActionExecuteDialog *ui;

    // QDialog interface
public slots:
    void accept() override;
};

#endif // ACTIONEXECUTEDIALOG_H
