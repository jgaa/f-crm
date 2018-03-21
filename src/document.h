#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <array>

#include <QIcon>
#include <QString>

struct Document
{
    enum class Type {
        NOTE, // Internal in the database
        EMAIL,
        URL,
        FILE
    };

    static constexpr size_t type_enums{4};
    using type_enums_t = std::array<Type, type_enums>;

    static const QIcon& typeIcon(const Type value);
    static const QIcon& typeIcon(const int value);
    static Type toType(const int value);
    static const QString& typeName(const Type value);
    static const QString& typeName(const int value);
    static const type_enums_t& typeEnums();


    enum class Class
    {
        RESEARCH,
        NOTE,
        PROPOSAL,
        REQUEST,
        OFFER
    };

    static constexpr size_t class_enums{5};
    using class_enums_t = std::array<Class, class_enums>;

    static const QIcon& classIcon(const Class value);
    static const QIcon& classIcon(const int value);
    static Class toClass(const int value);
    static const QString& className(const Class value);
    static const QString& className(const int value);
    static const class_enums_t& classEnums();

    enum class Direction
    {
        INTERNAL,
        INCOMING,
        OUTGOING
    };

    static constexpr size_t direction_enums{3};
    using direction_enums_t = std::array<Direction, direction_enums>;

    static const QIcon& directionIcon(const Direction value);
    static const QIcon& directionIcon(const int value);
    static Direction toDirection(const int value);
    static const QString& directionName(const Direction value);
    static const QString& directionName(const int value);
    static const direction_enums_t& directionEnums();

    enum class Entity
    {
        CONTACT,
        PERSON,
        INTENT,
        ACTION
    };

    static constexpr size_t entity_enums{4};
    using entity_enums_t = std::array<Entity, entity_enums>;

    static const QIcon& entityIcon(const Entity value);
    static const QIcon& entityIcon(const int value);
    static Entity toEntity(const int value);
    static const QString& entityName(const Entity value);
    static const QString& entityName(const int value);
    static const entity_enums_t& entityEnums();

    static void open(Type type, QString value);
    static void openFile(QString path);
    static void openUrl(QString url);
    static Type deduceType(const QUrl& url);
};

#endif // DOCUMENT_H
