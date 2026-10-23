#pragma once

#include <Arduino.h>
#include <time.h>

class TimeService
{
public:
    bool begin();

    void update(
        uint32_t nowMs,
        bool networkConnected
    );

    bool isValid() const;
    bool consumeSyncEvent();

    time_t epochUtc() const;
    uint32_t syncUptimeMs() const;
    time_t syncEpochUtc() const;

    String iso8601Utc() const;

private:
    bool clockLooksValid() const;

    bool _configured = false;
    bool _valid = false;
    bool _syncEvent = false;

    uint32_t _syncUptimeMs = 0;
    time_t _syncEpochUtc = 0;
};
