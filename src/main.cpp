#include "logging.h"
#include "mainwindow.h"
#include "version.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

void initSettings() {

    QSettings settings;

    qDebug() << "Settings are in " << settings.fileName();

    auto data_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir(data_path).exists()) {
        qDebug() << "Creating path: " << data_path;
        QDir().mkpath(data_path);
    }

    switch(auto version = settings.value("version").toInt()) {
        case 0: { // First use

            qInfo() << "First use - initializing settings.";
            settings.setValue("version", 1);
        } break;
    default:
        qDebug() << "Settings are OK at version " << version;
    }

    if (settings.value("dbpath", "").toString().isEmpty()) {
        QString dbpath = data_path;
#ifdef QT_DEBUG
        dbpath += "/f-crm-debug.db";
#else
        dbpath += "/f-crm.db";
#endif
        settings.setValue("dbpath", dbpath);
    }

    if (settings.value("log-path", "").toString().isEmpty()) {
        QString logpath = data_path;
#ifdef QT_DEBUG
        logpath += "/f-crm-debug.log";
#else
        logpath += "/f-crm.log";
#endif
        settings.setValue("log-path", logpath);
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("TheLastViking");
    a.setOrganizationDomain("lastviking.eu");

#ifdef QT_DEBUG
    a.setApplicationName("f-crm-debug");
#else
    a.setApplicationName("f-crm");
#endif
    initSettings();

    Logging logger;
    qInstallMessageHandler(Logging::logMessageHandler);

    MainWindow w;
    try {
        w.initialize();
    } catch(const std::exception& ex) {
        qWarning() << "Caught exception during initialization: "
                   << ex.what();
        return -1;
    }

    w.show();

    return a.exec();
}
