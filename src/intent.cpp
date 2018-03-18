#include "src/intent.h"

using namespace std;


QIcon GetIntentTypeIcon(const IntentType type)
{
    return GetIntentTypeIcon(static_cast<int>(type));
}


QIcon GetIntentTypeIcon(const int type)
{
    static const array<QIcon, 1> icons {{
        QIcon(":/res/icons/intent_type_manual.svg"),
    }};

    return icons.at(static_cast<size_t>(type));
}

IntentType ToIntentType(const int type)
{
    return GetIntentTypeEnums().at(static_cast<size_t>(type));
}

const QString &GetIntentTypeName(const IntentType type)
{
    return GetIntentTypeName(static_cast<int>(type));
}

const QString &GetIntentTypeName(const int type)
{
    static const array<QString, 1> names {{
            "Manual",
    }};

    return names.at(static_cast<size_t>(type));
}

const std::array<IntentType, 1>& GetIntentTypeEnums()
{
    static const std::array<IntentType, 1> enums {{
            IntentType::MANUAL,
    }};

    return enums;
}




QIcon GetIntentStateIcon(const IntentState type)
{
    return GetIntentStateIcon(static_cast<int>(type));
}


QIcon GetIntentStateIcon(const int type)
{
    static const array<QIcon, 5> icons {{
        QIcon(":/res/icons/state_defined.svg"),
        QIcon(":/res/icons/state_progress.svg"),
        QIcon(":/res/icons/state_succeeded.svg"),
        QIcon(":/res/icons/state_failed.svg"),
        QIcon(":/res/icons/state_terminated.svg"),

    }};

    return icons.at(static_cast<size_t>(type));
}

IntentState ToIntentState(const int type)
{
    return GetIntentStateEnums().at(static_cast<size_t>(type));
}

const QString &GetIntentStateName(const IntentState type)
{
    return GetIntentStateName(static_cast<int>(type));
}

const QString &GetIntentStateName(const int type)
{
    static const array<QString, 5> names {{
            "Defined",
            "Progress",
            "Succeeded",
            "Failed",
            "Terminated"
    }};

    return names.at(static_cast<size_t>(type));
}

const std::array<IntentState, 5>& GetIntentStateEnums()
{
    static const std::array<IntentState, 5> enums {{
            IntentState::DEFINED,
            IntentState::PROGRESS,
            IntentState::SUCCEEDED,
            IntentState::FAILED,
            IntentState::TERMINATED
    }};

    return enums;
}
