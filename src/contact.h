#ifndef PERSON_H
#define PERSON_H

#include <array>

#include <QIcon>
#include <QString>

enum class ContactType {
    CORPORATION,
    INDIVID
};

enum class ContactGender {
    UNKNOWN,
    MALE,
    FEMALE
};

enum class ContactStatus {
    CANDIDATE,
    WATCHING,
    PROSPECT,
    CUSTOMER,
    LOST,
    BANNED,
    SHUT_DOWN // No longer in business
};

QIcon GetContactTypeIcon(const ContactType type);
QIcon GetContactTypeIcon(const int type);
ContactType ToContactType(const int type);

QIcon GetContactGenderIcon(const ContactGender type);
QIcon GetContactGenderIcon(const int type);
ContactGender ToContactGender(const int type);
const QString& GetContactGenderName(const ContactGender type);
const QString& GetContactGenderName(const int type);
const std::array<ContactGender, 3>& GetContactGenderEnums();

QIcon GetContactStatusIcon(const ContactStatus type);
QIcon GetContactStatusIcon(const int type);
ContactStatus ToContactStatus(const int type);
const QString& GetContactStatusName(const ContactStatus type);
const QString& GetContactStatusName(const int type);
const std::array<ContactStatus, 7>& GetContactStatusEnums();


#endif // PERSON_H
