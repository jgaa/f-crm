#include "src/favoritesdialog.h"
#include "ui_favoritesdialog.h"

FavoritesDialog::FavoritesDialog(int row, int stars, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FavoritesDialog), row_{row}
{
    ui->setupUi(this);

    for(auto i = 0; i <= 5; i++) {
        ui->stars->insertItem(i,
                              new QListWidgetItem(
                                  QIcon(QStringLiteral(":/res/icons/%1star").arg(i)),
                                  "", ui->stars, 0));
    }

    ui->stars->setCurrentRow(stars);
}


FavoritesDialog::~FavoritesDialog()
{
    delete ui;
}


void FavoritesDialog::accept()
{
    emit setStars(row_, ui->stars->currentRow());
    QDialog::accept();
}
