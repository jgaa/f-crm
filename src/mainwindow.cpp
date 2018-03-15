#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
//    QIcon appicon(":res/icons/whid.svg");
//    setWindowIcon(appicon);

    QSettings settings;

    if (settings.value("restore-window-state", true).toBool()) {
        restoreGeometry(settings.value("windowGeometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
    }

}
