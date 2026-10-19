#pragma once

#include <Arduino.h>

enum class NodeLabRole : uint8_t
{
    Inspector = 0,
    Stub,
    Scenario
};

enum class CommsLabAction : uint8_t
{
    Status = 0,
    LedOff,
    LedSlow,
    LedFast,
    LedTriple,
    Relay1,
    Relay2,
    Relay3,
    Relay4,
    StubGardenDry,
    StubGardenHumid,
    StubGardenDark,
    Back,
    Count
};

inline NodeLabRole commsLabActionRole(
    CommsLabAction action
)
{
    switch (action)
    {
        case CommsLabAction::StubGardenDry:
        case CommsLabAction::StubGardenHumid:
        case CommsLabAction::StubGardenDark:
            return NodeLabRole::Stub;

        default:
            return NodeLabRole::Inspector;
    }
}

inline const char* nodeLabRoleName(
    NodeLabRole role
)
{
    switch (role)
    {
        case NodeLabRole::Inspector: return "INSPECTOR";
        case NodeLabRole::Stub: return "STUB";
        case NodeLabRole::Scenario: return "SCENARIO";
        default: return "UNKNOWN";
    }
}

inline const char* commsLabActionName(
    CommsLabAction action
)
{
    switch (action)
    {
        case CommsLabAction::Status: return "STATUS";
        case CommsLabAction::LedOff: return "LED OFF";
        case CommsLabAction::LedSlow: return "LED SLOW";
        case CommsLabAction::LedFast: return "LED FAST";
        case CommsLabAction::LedTriple: return "LED TRIPLE";
        case CommsLabAction::Relay1: return "RELAY 1";
        case CommsLabAction::Relay2: return "RELAY 2";
        case CommsLabAction::Relay3: return "RELAY 3";
        case CommsLabAction::Relay4: return "RELAY 4";
        case CommsLabAction::StubGardenDry: return "GDN DRY";
        case CommsLabAction::StubGardenHumid: return "GDN HUMID";
        case CommsLabAction::StubGardenDark: return "GDN DARK";
        case CommsLabAction::Back: return "BACK";
        case CommsLabAction::Count:
        default: return "UNKNOWN";
    }
}
