#pragma once

#include <Arduino.h>
#include <esp_now.h>
#include <esp_arduino_version.h>

#include "CommsLabTypes.h"

class CommsLabService
{
public:
    bool begin(
        uint8_t offlineFallbackChannel
    );

    void update(
        uint32_t nowMs
    );

    bool request(
        CommsLabAction action,
        uint32_t nowMs
    );

    bool consumeChanged();

    bool isReady() const;
    bool peerSeen() const;
    bool awaitingResponse() const;
    bool lastResponseOk() const;
    bool remoteRelayClicksEnabled() const;

    uint8_t channel() const;
    uint32_t lastRequestId() const;

    const char* localMac() const;
    const char* peerMac() const;
    const char* lastOperation() const;
    const char* lastState() const;
    const char* lastReason() const;

    String stateJson() const;

private:
    struct PendingRx
    {
        bool pending = false;
        uint8_t sourceMac[6] = {0};
        size_t length = 0;
        uint8_t data[241] = {0};
    };

    bool sendRequest(
        CommsLabAction action,
        uint32_t nowMs
    );

    void processIncoming(
        uint32_t nowMs
    );

    void enqueueReceived(
        const uint8_t* sourceMac,
        const uint8_t* data,
        int length
    );

    bool takePending(
        PendingRx& output
    );

    void setChanged();

    static void formatMac(
        const uint8_t* mac,
        char* output,
        size_t outputSize
    );

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    static void onDataRecv(
        const esp_now_recv_info_t* info,
        const uint8_t* data,
        int length
    );
#else
    static void onDataRecv(
        const uint8_t* mac,
        const uint8_t* data,
        int length
    );
#endif

    static CommsLabService* _instance;

    PendingRx _pendingRx;
    portMUX_TYPE _rxMux = portMUX_INITIALIZER_UNLOCKED;

    bool _ready = false;
    bool _peerSeen = false;
    bool _awaitingResponse = false;
    bool _lastResponseOk = false;
    bool _remoteRelayClicksEnabled = false;
    bool _changed = false;

    uint32_t _nextRequestId = 1;
    uint32_t _lastRequestId = 0;
    uint32_t _requestSentAtMs = 0;

    char _localMac[18] = "--";
    char _peerMac[18] = "--";
    char _lastOperation[28] = "NONE";
    char _lastState[24] = "IDLE";
    char _lastReason[24] = "NONE";
};
