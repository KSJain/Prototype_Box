#pragma once

#include <Arduino.h>

class NetworkService
{
public:
    bool begin(
        uint32_t nowMs
    );

    void update(
        uint32_t nowMs
    );

    bool enabled() const;
    bool connected() const;
    bool consumeConnected();
    bool consumeDisconnected();

    uint8_t channel() const;
    int32_t rssi() const;

    String localIp() const;
    const char* ssid() const;

private:
    void requestConnection(
        uint32_t nowMs
    );

    bool _enabled = false;
    bool _wasConnected = false;
    bool _connectedEvent = false;
    bool _disconnectedEvent = false;
    uint32_t _lastConnectAttemptMs = 0;
};
