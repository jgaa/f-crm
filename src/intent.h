#ifndef INTENTS_H
#define INTENTS_H

#include <array>

#include <QIcon>
#include <QString>

enum class IntentType {
    MANUAL
};

enum class IntentState {
    DEFINED,
    PROGRESS,
    SUCCEEDED,
    FAILED,
    TERMINATED
};


QIcon GetIntentTypeIcon(const IntentType type);
QIcon GetIntentTypeIcon(const int type);
IntentType ToIntentType(const int type);
const QString& GetIntentTypeName(const IntentType type);
const QString& GetIntentTypeName(const int type);
const std::array<IntentType, 1>& GetIntentTypeEnums();


QIcon GetIntentStateIcon(const IntentState type);
QIcon GetIntentStateIcon(const int type);
IntentState ToIntentState(const int type);
const QString& GetIntentStateName(const IntentState type);
const QString& GetIntentStateName(const int type);
const std::array<IntentState, 5>& GetIntentStateEnums();


#endif // INTENTS_H
