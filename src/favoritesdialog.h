#ifndef FAVORITESDIALOG_H
#define FAVORITESDIALOG_H

#include <QDialog>

namespace Ui {
class FavoritesDialog;
}

class FavoritesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FavoritesDialog(int row, int stars, QWidget *parent = 0);
    ~FavoritesDialog();

signals:
    void setStars(const int row, const int stars);

private:
    Ui::FavoritesDialog *ui;
    const int row_;

    // QDialog interface
public slots:
    void accept() override;
};

#endif // FAVORITESDIALOG_H
