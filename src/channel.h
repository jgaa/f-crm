#ifndef CHANNEL_H
#define CHANNEL_H

#include <array>

#include <QIcon>

enum class ChannelType {
    OTHER,
    WEB,
    EMAIL,
    PHONE,
    MOBILE,
    SKYPE,
    LINKEDIN,
    REDDIT,
    FACEBOOK,
    GITHUB
};

QIcon GetChannelStatusIcon(const ChannelType type);
QIcon GetChannelStatusIcon(const int type);
QString GetChannelTypeName(const ChannelType type);
QString GetChannelTypeName(const int type);
const std::array<ChannelType, 10>& GetChannelTypeEnums();
ChannelType ToChannelType(const int type);

#endif // CHANNEL_H
