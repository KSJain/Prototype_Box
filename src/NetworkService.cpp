#include "NetworkService.h"

#include <WiFi.h>
#include <esp_wifi.h>

#include "config/NetworkConfig.h"

bool NetworkService::begin(
    uint32_t nowMs
)
{
    _enabled =
        NetworkConfig::ENABLED &&
        NetworkConfig::SSID[0] != '\0';

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);

    if (!_enabled)
    {
        Serial.println(
            "Network: disabled (no local config)"
        );

        return false;
    }

    requestConnection(nowMs);

    Serial.print("Network: connecting to ");
    Serial.println(NetworkConfig::SSID);

    return true;
}

void NetworkService::update(
    uint32_t nowMs
)
{
    if (!_enabled)
    {
        return;
    }

    const bool isConnected =
        WiFi.status() == WL_CONNECTED;

    if (isConnected != _wasConnected)
    {
        _wasConnected = isConnected;

        if (isConnected)
        {
            _connectedEvent = true;

            Serial.print("Network: connected, IP ");
            Serial.print(WiFi.localIP());
            Serial.print(", channel ");
            Serial.println(channel());
        }
        else
        {
            _disconnectedEvent = true;
            Serial.println("Network: disconnected");
        }
    }

    if (
        !isConnected &&
        nowMs - _lastConnectAttemptMs >=
            NetworkConfig::RECONNECT_INTERVAL_MS
    )
    {
        requestConnection(nowMs);
    }
}

bool NetworkService::enabled() const
{
    return _enabled;
}

bool NetworkService::connected() const
{
    return
        _enabled &&
        WiFi.status() == WL_CONNECTED;
}

bool NetworkService::consumeConnected()
{
    const bool value = _connectedEvent;
    _connectedEvent = false;
    return value;
}

bool NetworkService::consumeDisconnected()
{
    const bool value = _disconnectedEvent;
    _disconnectedEvent = false;
    return value;
}

uint8_t NetworkService::channel() const
{
    uint8_t primary = 0;
    wifi_second_chan_t secondary =
        WIFI_SECOND_CHAN_NONE;

    if (
        esp_wifi_get_channel(
            &primary,
            &secondary
        ) != ESP_OK
    )
    {
        return 0;
    }

    return primary;
}

int32_t NetworkService::rssi() const
{
    return connected()
        ? WiFi.RSSI()
        : 0;
}

String NetworkService::localIp() const
{
    return connected()
        ? WiFi.localIP().toString()
        : String("0.0.0.0");
}

const char* NetworkService::ssid() const
{
    return _enabled
        ? NetworkConfig::SSID
        : "";
}

void NetworkService::requestConnection(
    uint32_t nowMs
)
{
    _lastConnectAttemptMs = nowMs;

    WiFi.begin(
        NetworkConfig::SSID,
        NetworkConfig::PASSWORD
    );
}
