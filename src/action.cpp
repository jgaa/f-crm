#include "src/action.h"

using namespace std;

QIcon GetActionStateIcon(const ActionState type)
{
    return GetActionStateIcon(static_cast<int>(type));
}


QIcon GetActionStateIcon(const int type)
{
    static const array<QIcon, 7> icons {{
        QIcon(":/res/icons/action_waiting.svg"),
        QIcon(":/res/icons/action_ready.svg"),
        QIcon(":/res/icons/action_blocked.svg"),
        QIcon(":/res/icons/action_on_hold.svg"),
        QIcon(":/res/icons/action_done.svg"),
        QIcon(":/res/icons/action_cancelled.svg"),
        QIcon(":/res/icons/action_failed.svg")
    }};

    return icons.at(static_cast<size_t>(type));
}

ActionState ToActionState(const int type)
{
    return GetActionStateEnums().at(static_cast<size_t>(type));
}

const QString &GetActionStateName(const ActionState type)
{
    return GetActionStateName(static_cast<int>(type));
}

const QString &GetActionStateName(const int type)
{
    static const array<QString, 7> names {{
            "Waiting",
            "Open",
            "Blocked",
            "On hold",
            "Done",
            "Cancelled",
            "Failed"
    }};

    return names.at(static_cast<size_t>(type));
}

const std::array<ActionState, 7>& GetActionStateEnums()
{
    static const std::array<ActionState, 7> enums {{
        ActionState::WAITING,
        ActionState::OPEN,
        ActionState::BLOCKED,
        ActionState::ON_HOLD,
        ActionState::DONE,
        ActionState::CANCELLED,
        ActionState::FAILED
    }};

    return enums;
}



QIcon GetActionTypeIcon(const ActionType type)
{
    return GetActionTypeIcon(static_cast<int>(type));
}


QIcon GetActionTypeIcon(const int type)
{
    static const array<QIcon, 3> icons {{
        QIcon(":/res/icons/action_type_task.svg"),
        QIcon(":/res/icons/action_type_channel.svg"),
        QIcon(":/res/icons/action_type_meeting.svg"),
    }};

    return icons.at(static_cast<size_t>(type));
}

ActionType ToActionType(const int type)
{
    return GetActionTypeEnums().at(static_cast<size_t>(type));
}

const QString &GetActionTypeName(const ActionType type)
{
    return GetActionTypeName(static_cast<int>(type));
}

const QString &GetActionTypeName(const int type)
{
    static const array<QString, 3> names {{
            "Task",
            "Channel",
            "Meeting",
    }};

    return names.at(static_cast<size_t>(type));
}

const std::array<ActionType, 3>& GetActionTypeEnums()
{
    static const std::array<ActionType, 3> enums {{
        ActionType::TASK,
        ActionType::CHANNEL,
        ActionType::MEETING,
    }};

    return enums;
}


