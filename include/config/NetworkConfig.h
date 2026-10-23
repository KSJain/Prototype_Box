#pragma once

// Local credentials intentionally do not belong in Git.
// Copy NetworkConfig.example.h -> NetworkConfig.local.h and edit locally.
#if __has_include("NetworkConfig.local.h")
#include "NetworkConfig.local.h"
#else
namespace NetworkConfig
{
    constexpr bool ENABLED = false;
    constexpr char SSID[] = "DEV_KSJ_007";
    constexpr char PASSWORD[] = "om!246&ultra";

    constexpr uint32_t RECONNECT_INTERVAL_MS = 15000;
    constexpr uint8_t OFFLINE_ESPNOW_CHANNEL = 1;
}
#endif