#ifndef ACTION_H
#define ACTION_H

#include <array>

#include <QIcon>
#include <QString>

enum class ActionState {
    WAITING,
    OPEN,
    BLOCKED,
    ON_HOLD,
    DONE,
    CANCELLED,
    FAILED
};

enum class ActionType {
    TASK,
    CHANNEL,
    MEETING
};

QIcon GetActionStateIcon(const ActionState type);
QIcon GetActionStateIcon(const int type);
ActionState ToActionState(const int type);
const QString& GetActionStateName(const ActionState type);
const QString& GetActionStateName(const int type);
const std::array<ActionState, 7>& GetActionStateEnums();


QIcon GetActionTypeIcon(const ActionType type);
QIcon GetActionTypeIcon(const int type);
ActionType ToActionType(const int type);
const QString& GetActionTypeName(const ActionType type);
const QString& GetActionTypeName(const int type);
const std::array<ActionType, 3>& GetActionTypeEnums();

#endif // ACTION_H
