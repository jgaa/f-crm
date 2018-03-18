#include <array>

#include "src/contact.h"

using namespace std;

QIcon GetContactTypeIcon(const ContactType type)
{
    return GetContactTypeIcon(static_cast<int>(type));
}

QIcon GetContactTypeIcon(const int type)
{
    static const array<QIcon, 2> icons{{
        QIcon(":/res/icons/company.svg"),
        QIcon(":/res/icons/person.svg")
    }};

    return icons.at(static_cast<size_t>(type));
}

ContactType ToContactType(const int type)
{
    static const array<ContactType, 2> types {{
        ContactType::CORPORATION,
        ContactType::INDIVID
    }};

    return types.at(static_cast<size_t>(type));
}

QIcon GetContactGenderIcon(const ContactGender type)
{
    return GetContactGenderIcon(static_cast<int>(type));
}

QIcon GetContactGenderIcon(const int type)
{
    static const array<QIcon, 3> icons{{
        QIcon(":/res/icons/gender_unknown.svg"),
        QIcon(":/res/icons/gender_male.svg"),
        QIcon(":/res/icons/gender_female.svg")
    }};

    return icons.at(static_cast<size_t>(type));
}

ContactGender ToContactGender(const int type)
{
    static const array<ContactGender, 3> types {{
        ContactGender::UNKNOWN,
        ContactGender::MALE,
        ContactGender::FEMALE
    }};

    return types.at(static_cast<size_t>(type));
}

const QString &GetContactGenderName(const ContactGender type)
{
    return GetContactGenderName(static_cast<int>(type));
}

const QString &GetContactGenderName(const int type)
{
    static const array<QString, 7> names {{
        "Unknown",
        "Male",
        "Female"
    }};

    return names.at(static_cast<size_t>(type));
}



QIcon GetContactStatusIcon(const ContactStatus type)
{
    return GetContactStatusIcon(static_cast<int>(type));
}


QIcon GetContactStatusIcon(const int type)
{
    static const array<QIcon, 7> icons{{
        QIcon(":/res/icons/status_candidate.svg"),
        QIcon(":/res/icons/status_watching.svg"),
        QIcon(":/res/icons/status_prospect.svg"),
        QIcon(":/res/icons/status_customer.svg"),
        QIcon(":/res/icons/status_lost.svg"),
        QIcon(":/res/icons/status_banned.svg"),
        QIcon(":/res/icons/status_shut_down.svg")
    }};

    return icons.at(static_cast<size_t>(type));
}

ContactStatus ToContactStatus(const int type)
{
    return GetContactStatusEnums().at(static_cast<size_t>(type));
}

const array<ContactStatus, 7>& GetContactStatusEnums()
{
    static const array<ContactStatus, 7> enums {{
        ContactStatus::CANDIDATE,
        ContactStatus::WATCHING,
        ContactStatus::PROSPECT,
        ContactStatus::CUSTOMER,
        ContactStatus::LOST,
        ContactStatus::BANNED,
        ContactStatus::SHUT_DOWN
    }};

    return enums;
}

const QString &GetContactStatusName(const ContactStatus type)
{
    return GetContactStatusName(static_cast<int>(type));
}

const QString &GetContactStatusName(const int type)
{
    static const array<QString, 7> names {{
        "Candidate",
        "Watching",
        "Prospect",
        "Customer",
        "Lost",
        "Banned",
        "Shut_down"
    }};

    return names.at(static_cast<size_t>(type));
}

const std::array<ContactGender, 3> &GetContactGenderEnums()
{
    static const array<ContactGender, 3> enums {{
            ContactGender::UNKNOWN,
            ContactGender::MALE,
            ContactGender::FEMALE
    }};

    return enums;
}
