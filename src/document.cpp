#include "src/document.h"

#include <QDesktopServices>
#include <QProcess>
#include <QSettings>
#include <QUrl>
#include <QDebug>

using namespace std;

/////////// Type //////////

const QIcon &Document::typeIcon(const Document::Type type)
{
    return typeIcon(static_cast<int>(type));
}

const QIcon &Document::typeIcon(const int type)
{
    static const array<QIcon, type_enums> icons {{
        QIcon(":/res/icons/note.svg"),
        QIcon(":/res/icons/mail.svg"),
        QIcon(":/res/icons/external-link.svg"),
        QIcon(":/res/icons/file.svg"),
    }};

    return icons.at(static_cast<size_t>(type));
}

Document::Type Document::toType(const int type)
{
    return typeEnums().at(static_cast<size_t>(type));
}

const QString &Document::typeName(const Document::Type type)
{
    return typeName(static_cast<int>(type));
}

const QString &Document::typeName(const int type)
{
    static const array<QString, type_enums> names {{
            "Note",
            "Email",
            "Url",
            "File"
    }};

    return names.at(static_cast<size_t>(type));
}

const Document::type_enums_t &Document::typeEnums()
{
    static const std::array<Document::Type, type_enums> enums {{
        Type::NOTE,
        Type::EMAIL,
        Type::URL,
        Type::FILE
    }};

    return enums;
}


/////////// Class //////////

const QIcon &Document::classIcon(const Document::Class value)
{
    return classIcon(static_cast<int>(value));
}

const QIcon &Document::classIcon(const int value)
{
    static const array<QIcon, class_enums> icons {{
        QIcon(":/res/icons/research.svg"),
        QIcon(":/res/icons/note.svg"),
        QIcon(":/res/icons/proposal.svg"),
        QIcon(":/res/icons/request.svg"),
        QIcon(":/res/icons/offer.svg"),
    }};

    return icons.at(static_cast<size_t>(value));
}

Document::Class Document::toClass(const int value)
{
    return classEnums().at(static_cast<size_t>(value));
}

const QString &Document::className(const Document::Class value)
{
    return className(static_cast<int>(value));
}

const QString &Document::className(const int value)
{
    static const array<QString, class_enums> names {{
        "Research",
        "Note",
        "Proposal",
        "Request",
        "Offer"
    }};

    return names.at(static_cast<size_t>(value));
}

const Document::class_enums_t &Document::classEnums()
{
    static const std::array<Document::Class, class_enums> enums {{
        Class::RESEARCH,
        Class::NOTE,
        Class::PROPOSAL,
        Class::REQUEST,
        Class::OFFER
    }};

    return enums;
}


/////////// Direction //////////

const QIcon &Document::directionIcon(const Document::Direction value)
{
    return directionIcon(static_cast<int>(value));
}

const QIcon &Document::directionIcon(const int value)
{
    static const array<QIcon, direction_enums> icons {{
        QIcon(":/res/icons/internal.svg"),
        QIcon(":/res/icons/incoming.svg"),
        QIcon(":/res/icons/outgoing.svg"),
    }};

    return icons.at(static_cast<size_t>(value));
}

Document::Direction Document::toDirection(const int value)
{
    return directionEnums().at(static_cast<size_t>(value));
}

const QString &Document::directionName(const Document::Direction value)
{
    return directionName(static_cast<int>(value));
}

const QString &Document::directionName(const int value)
{
    static const array<QString, direction_enums> names {{
        "Internal",
        "Incoming",
        "Outgoing",
    }};

    return names.at(static_cast<size_t>(value));
}

const Document::direction_enums_t &Document::directionEnums()
{
    static const std::array<Document::Direction, direction_enums> enums {{
            Direction::INTERNAL,
            Direction::INCOMING,
            Direction::OUTGOING
    }};

    return enums;
}


/////////// Entity //////////

const QIcon &Document::entityIcon(const Document::Entity value)
{
    return entityIcon(static_cast<int>(value));
}

const QIcon &Document::entityIcon(const int value)
{
    static const array<QIcon, entity_enums> icons {{
        QIcon(":/res/icons/company.svg"),
        QIcon(":/res/icons/person.svg"),
        QIcon(":/res/icons/intent.svg"),
        QIcon(":/res/icons/activity.svg")
    }};

    return icons.at(static_cast<size_t>(value));
}

Document::Entity Document::toEntity(const int value)
{
    return entityEnums().at(static_cast<size_t>(value));
}

const QString &Document::entityName(const Document::Entity value)
{
    return entityName(static_cast<int>(value));
}

const QString &Document::entityName(const int value)
{
    static const array<QString, entity_enums> names {{
        "Contact",
        "Person",
        "Intent",
        "Action"
    }};

    return names.at(static_cast<size_t>(value));
}

const Document::entity_enums_t &Document::entityEnums()
{
    static const std::array<Document::Entity, entity_enums> enums {{
        Entity::CONTACT,
        Entity::PERSON,
        Entity::INTENT,
        Entity::ACTION
}};

    return enums;
}

void Document::open(Document::Type type, QString value)
{
    if (type == Document::Type::FILE || type == Document::Type::EMAIL) {

        QSettings settings;
        const auto mailapp = settings.value("mailapp", "").toString();
        if (value.startsWith("imap:") && !mailapp.isEmpty()) {
            QProcess::startDetached(mailapp, {value});
        } else {
            Document::openFile(value);
        }
    } else if (type == Document::Type::URL) {
        Document::openUrl(value);
    }
}

void Document::openFile(QString path)
{
    if (path.isEmpty()) {
        return;
    }

    if (!path.startsWith("file://")) {
        path = QStringLiteral("file://%1").arg(path);
    }

    QDesktopServices::openUrl(QUrl(path));
}

void Document::openUrl(QString url)
{
    if (url.isEmpty()) {
        return;
    }

    if (!url.contains(':')) {
        url = QStringLiteral("https://%1").arg(url);
    }

    QDesktopServices::openUrl(QUrl(url));
}

Document::Type Document::deduceType(const QUrl &url)
{
    const auto scheme = url.scheme();
    if (scheme.isEmpty())
        return Type::NOTE;
    if (scheme == "imap" || scheme == "mailto")
        return Type::EMAIL;
    if (scheme == "file")
        return Type::FILE;
    return Type::URL;
}
