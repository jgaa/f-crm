#include "src/database.h"

#include <QDebug>
#include <QFileInfo>


Database::Database(QObject *parent)
    : QObject(parent)
{
    static const auto DRIVER{QStringLiteral("QSQLITE")};

    QSettings settings;

    const auto dbpath = settings.value("dbpath").toString();
    const bool new_database = (dbpath == ":memory:") || (!QFileInfo(dbpath).isFile());

    if(!QSqlDatabase::isDriverAvailable(DRIVER)) {
        throw Error("Missing sqlite3 support");
    }

    db_ = QSqlDatabase::addDatabase(DRIVER);
    db_.setDatabaseName(dbpath);

    if (!db_.open()) {
        qWarning() << "Failed to open database: " << dbpath;
        throw Error("Failed to open database");
    }

    if (new_database) {
        qInfo() << "Creating new database at location: " << dbpath;
        createDatabase();
    }

    QSqlQuery query("SELECT * FROM f_crm");
    if (!query.next()) {
        throw Error("Missing configuration record in database");
    }

    const auto dbver = query.value(FCRM_VERSION).toInt();
    qDebug() << "Database schema version is " << dbver;
    if (dbver != currentVersion) {
        qWarning() << "Database schema version is "
                   << dbver
                   << " while I expected " << currentVersion;
    }

}

Database::~Database()
{
    // Close the database and remove the connection to make our tests happy (no warnings).
    const auto name = db_.connectionName();
    db_.close();
    db_ = {};
    db_.removeDatabase(name);
}

void Database::createDatabase()
{
    db_.transaction();

    try {
        exec(R"(CREATE TABLE "f_crm" ( `version` INTEGER NOT NULL))");
        exec(R"(CREATE TABLE "contact" ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `contact` INTEGER, `created_date` INTEGER, `last_activity_date` INTEGER, `name` TEXT NOT NULL, `first_name` TEXT, `middle_name` TEXT, `last_name` TEXT, `gender` INTEGER DEFAULT 0, `type` INTEGER DEFAULT 0, `status` INTEGER DEFAULT 0, `notes` TEXT, `stars` INTEGER, `favourite` INTEGER DEFAULT 0, `address1` TEXT, `address2` TEXT, `city` TEXT, `postcode` TEXT, `country` TEXT, FOREIGN KEY(`contact`) REFERENCES `contact`(`id`) ))");
        exec(R"(CREATE TABLE "channel" ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `contact` INTEGER NOT NULL, `type` INTEGER NOT NULL DEFAULT 0, `value` TEXT, `verified` INTEGER NOT NULL DEFAULT 0, FOREIGN KEY(`contact`) REFERENCES `contact`(`id`) ))");
        exec(R"(CREATE TABLE `intent` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `contact` INTEGER NOT NULL, `type` INTEGER NOT NULL DEFAULT 0, `state` INTEGER NOT NULL DEFAULT 0, `abstract` TEXT, `notes` TEXT, FOREIGN KEY(`contact`) REFERENCES contact(id) ))");
        exec(R"(CREATE TABLE "action" ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `sequence` INTEGER NOT NULL, `contact` INTEGER NOT NULL, `intent` INTEGER, `person` INTEGER, `state` INTEGER NOT NULL DEFAULT 0, `type` INTEGER, `channel_type` INTEGER, `name` TEXT, `created_date` INTEGER NOT NULL, `start_date` INTEGER, `due_date` INTEGER, `desired_outcome` TEXT, `notes` TEXT, FOREIGN KEY(`contact`) REFERENCES `contact`(`id`), FOREIGN KEY(`intent`) REFERENCES `intent`(`id`), FOREIGN KEY(`person`) REFERENCES `contact`(`id`) ))");
        exec(R"(CREATE TABLE "document" ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `contact` INTEGER NOT NULL, `person` INTEGER, `intent` INTEGER, `activity` INTEGER, `type` INTEGER NOT NULL DEFAULT 0, `cls` INTEGER NOT NULL, `direction` INTEGER NOT NULL, `entity` INTEGER NOT NULL, `name` TEXT NOT NULL, `notes` TEXT, `added_date` INTEGER NOT NULL, `file_date` INTEGER, `location` TEXT, `content` BLOB, FOREIGN KEY(`contact`) REFERENCES `contact`(`id`), FOREIGN KEY(`person`) REFERENCES `contact`(`id`), FOREIGN KEY(`intent`) REFERENCES `intent`(`id`), FOREIGN KEY(`activity`) REFERENCES `activity`(`id`) ))");
        exec(R"(CREATE TABLE "log" ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `type` INTEGER NOT NULL, `date` INTEGER NOT NULL, `contact` INTEGER, `person` INTEGER, `intent` INTEGER, `channel` INTEGER, `activity` INTEGER, `document` INTEGER, `text` TEXT NOT NULL, FOREIGN KEY(`contact`) REFERENCES `contact`(`id`), FOREIGN KEY(`person`) REFERENCES `contact`(`id`), FOREIGN KEY(`intent`) REFERENCES `intent`(`id`), FOREIGN KEY(`channel`) REFERENCES `channel`(`id`), FOREIGN KEY(`activity`) REFERENCES `activity`(`id`), FOREIGN KEY(`document`) REFERENCES document(id) ))");

        QSqlQuery query(db_);
        query.prepare("INSERT INTO f_crm (version) VALUES (:version)");
        query.bindValue(":version", currentVersion);
        if(!query.exec()) {
            throw Error(QStringLiteral("Failed to initialize database: %1").arg(query.lastError().text()));
        }

    } catch(const std::exception&) {
        db_.rollback();
        throw;
    }

    db_.commit();
}

void Database::exec(const char *sql)
{
    QSqlQuery query(db_);
    query.exec(sql);
    if (query.lastError().type() != QSqlError::NoError) {
        throw Error(QStringLiteral("SQL query failed: %1").arg(query.lastError().text()));
    }
}

