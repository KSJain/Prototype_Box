#include "TimeService.h"

namespace TimeServiceConfig
{
    constexpr char NTP_1[] = "time.cloudflare.com";
    constexpr char NTP_2[] = "pool.ntp.org";
    constexpr char NTP_3[] = "time.google.com";

    // Any epoch after 2024-01-01 is sufficient evidence that SNTP/system
    // wall clock has been initialized for this lab.
    constexpr time_t MINIMUM_VALID_EPOCH = 1704067200;
}

bool TimeService::begin()
{
    _configured = false;
    _valid = false;
    _syncEvent = false;
    _syncUptimeMs = 0;
    _syncEpochUtc = 0;

    Serial.println(
        "Time: waiting for network; monotonic clock remains authoritative"
    );

    return true;
}

void TimeService::update(
    uint32_t nowMs,
    bool networkConnected
)
{
    if (
        networkConnected &&
        !_configured
    )
    {
        // UTC internally: offset 0, daylight offset 0.
        configTime(
            0,
            0,
            TimeServiceConfig::NTP_1,
            TimeServiceConfig::NTP_2,
            TimeServiceConfig::NTP_3
        );

        _configured = true;

        Serial.println(
            "Time: NTP configured; sync remains non-blocking"
        );
    }

    const bool nowValid =
        clockLooksValid();

    if (
        nowValid &&
        !_valid
    )
    {
        _valid = true;
        _syncUptimeMs = nowMs;
        _syncEpochUtc = time(nullptr);
        _syncEvent = true;

        Serial.print("Time: UTC valid at uptime_ms=");
        Serial.print(_syncUptimeMs);
        Serial.print(", epoch=");
        Serial.println(
            static_cast<unsigned long>(
                _syncEpochUtc
            )
        );
    }
}

bool TimeService::isValid() const
{
    return _valid && clockLooksValid();
}

bool TimeService::consumeSyncEvent()
{
    const bool value = _syncEvent;
    _syncEvent = false;
    return value;
}

time_t TimeService::epochUtc() const
{
    return isValid()
        ? time(nullptr)
        : 0;
}

uint32_t TimeService::syncUptimeMs() const
{
    return _syncUptimeMs;
}

time_t TimeService::syncEpochUtc() const
{
    return _syncEpochUtc;
}

String TimeService::iso8601Utc() const
{
    if (!isValid())
    {
        return "";
    }

    const time_t now = time(nullptr);
    struct tm utc{};

    if (gmtime_r(&now, &utc) == nullptr)
    {
        return "";
    }

    char buffer[25];

    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%dT%H:%M:%SZ",
        &utc
    );

    return String(buffer);
}

bool TimeService::clockLooksValid() const
{
    return
        time(nullptr) >=
        TimeServiceConfig::MINIMUM_VALID_EPOCH;
}
