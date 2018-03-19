#include <array>

#include "channel.h"


using namespace std;

QIcon GetChannelStatusIcon(const ChannelType type)
{
    return GetChannelStatusIcon(static_cast<int>(type));
}

QIcon GetChannelStatusIcon(const int type)
{
    static const array<QIcon, 10> icons{{
        QIcon(":/res/icons/ch_type_other.svg"),
        QIcon(":/res/icons/ch_type_web.svg"),
        QIcon(":/res/icons/mail.svg"),
        QIcon(":/res/icons/phone.svg"),
        QIcon(":/res/icons/mobile.svg"),
        QIcon(":/res/icons/skype.svg"),
        QIcon(":/res/icons/linkedin.svg"),
        QIcon(":/res/icons/reddit.svg"),
        QIcon(":/res/icons/facebook.svg"),
        QIcon(":/res/icons/github.svg")
    }};

    return icons.at(static_cast<size_t>(type));
}

ChannelType ToChannelType(const int type)
{
    static const array<ChannelType, 10> types {{
        ChannelType::OTHER,
        ChannelType::WEB,
        ChannelType::EMAIL,
        ChannelType::PHONE,
        ChannelType::MOBILE,
        ChannelType::SKYPE,
        ChannelType::LINKEDIN,
        ChannelType::REDDIT,
        ChannelType::FACEBOOK,
        ChannelType::GITHUB
    }};

    return types.at(static_cast<size_t>(type));
}

QString GetChannelTypeName(const ChannelType type)
{
    return GetChannelTypeName(static_cast<int>(type));
}

QString GetChannelTypeName(const int type)
{
    static const array<QString, 10> names {{
            "Other",
            "Web",
            "Email",
            "Phone",
            "Mobile",
            "Skype",
            "Linkedin",
            "Reddit",
            "Facebook",
            "Github",
    }};

    return names.at(static_cast<size_t>(type));
}

const std::array<ChannelType, 10> &GetChannelTypeEnums()
{
    static const std::array<ChannelType, 10> enums {{
        ChannelType::OTHER,
        ChannelType::WEB,
        ChannelType::EMAIL,
        ChannelType::PHONE,
        ChannelType::MOBILE,
        ChannelType::SKYPE,
        ChannelType::LINKEDIN,
        ChannelType::REDDIT,
        ChannelType::FACEBOOK,
        ChannelType::GITHUB
    }};

    return enums;
}
