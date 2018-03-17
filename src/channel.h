#ifndef CHANNEL_H
#define CHANNEL_H

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
    GITHUB,
};

QIcon GetChannelStatusIcon(const ChannelType type);
QIcon GetChannelStatusIcon(const int type);
ChannelType ToChannelType(const int type);

#endif // CHANNEL_H
