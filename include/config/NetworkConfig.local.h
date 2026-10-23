#pragma once

namespace NetworkConfig
{
    constexpr bool ENABLED = true;

    constexpr char SSID[] =
        "YOUR_WIFI_SSID";

    constexpr char PASSWORD[] =
        "YOUR_WIFI_PASSWORD";

    // Wi-Fi association is always opportunistic and non-blocking.
    constexpr uint32_t RECONNECT_INTERVAL_MS = 15000;

    // Used only while the node is not associated with a Wi-Fi AP.
    // Once associated, ESP-NOW follows the AP/current radio channel.
    constexpr uint8_t OFFLINE_ESPNOW_CHANNEL = 1;
}
