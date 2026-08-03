#include "StorageService.h"

namespace StorageServiceConfig
{
    constexpr uint8_t CS = 5;
    constexpr uint8_t SCK = 18;
    constexpr uint8_t MISO = 19;
    constexpr uint8_t MOSI = 23;

    constexpr char SESSION_PREFIX[] =
        "PB-";

    constexpr uint32_t TELEMETRY_INTERVAL_MS =
        5000;

    constexpr uint16_t MAXIMUM_SESSIONS =
        100;

    constexpr uint64_t MAXIMUM_SESSION_BYTES =
        2ULL * 1024ULL * 1024ULL;

    constexpr uint64_t MAXIMUM_TOTAL_BYTES =
        100ULL * 1024ULL * 1024ULL;
}

StorageService::StorageService(
    SPIClass& spi,
    KSJ::SDStorage& storage
)
    : _spi(spi),
      _storage(storage),
      _logger(storage),
      _sequence(storage),
      _inspector(storage),
      _retention(storage)
{
}

bool StorageService::begin(
    const char* firmwareVersion,
    const char* boardName
)
{
    _ready = false;
    _sessionNumber = 0;
    _sessionId = "";

    _spi.begin(
        StorageServiceConfig::SCK,
        StorageServiceConfig::MISO,
        StorageServiceConfig::MOSI,
        StorageServiceConfig::CS
    );

    const KSJ::StorageResult mountResult =
        _storage.begin();

    Serial.print("Storage mount: ");
    Serial.println(
        mountResult
            ? "SUCCESS"
            : "FAILED"
    );

    if (!mountResult)
    {
        Serial.println(
            "Prototype Box continues without memory."
        );

        return false;
    }

    KSJ::RetentionPolicy policy;

    policy.maximumSessions =
        StorageServiceConfig::
            MAXIMUM_SESSIONS;

    policy.maximumSessionBytes =
        StorageServiceConfig::
            MAXIMUM_SESSION_BYTES;

    policy.maximumTotalBytes =
        StorageServiceConfig::
            MAXIMUM_TOTAL_BYTES;

    const KSJ::StorageResult retentionResult =
        _retention.apply(
            StorageServiceConfig::
                SESSION_PREFIX,
            policy,
            _health
        );

    Serial.print("Retention: ");
    Serial.println(
        retentionResult
            ? "SUCCESS"
            : "FAILED"
    );

    if (retentionResult)
    {
        printHealth();
    }

    const KSJ::StorageResult sequenceResult =
        _sequence.findNext(
            StorageServiceConfig::
                SESSION_PREFIX,
            _sessionNumber,
            _sessionId
        );

    Serial.print("Session sequence: ");
    Serial.println(
        sequenceResult
            ? "SUCCESS"
            : "FAILED"
    );

    if (!sequenceResult)
    {
        return false;
    }

    KSJ::SessionInspection previousInspection;
    String previousSessionId;

    const bool hasPreviousSession =
        _sessionNumber > 1;

    if (hasPreviousSession)
    {
        previousSessionId =
            makeSessionId(
                _sessionNumber - 1
            );

        const String previousPath =
            makeSessionPath(
                previousSessionId
            );

        const KSJ::StorageResult
            inspectionResult =
                _inspector.inspect(
                    previousPath.c_str(),
                    previousInspection
                );

        Serial.print(
            "Previous inspection: "
        );

        Serial.println(
            inspectionResult
                ? KSJ::
                    sessionInspectionStatusName(
                        previousInspection.status
                    )
                : "FAILED"
        );
    }

    KSJ::SessionInfo session;

    session.sessionId =
        _sessionId;

    session.bootCount =
        _sessionNumber;

    session.firmwareVersion =
        firmwareVersion != nullptr
            ? firmwareVersion
            : "UNKNOWN";

    session.boardName =
        boardName != nullptr
            ? boardName
            : "UNKNOWN";

    session.startedAtMs =
        millis();

    const KSJ::StorageResult sessionResult =
        _logger.begin(
            session
        );

    Serial.print("Session start: ");
    Serial.println(
        sessionResult
            ? "SUCCESS"
            : "FAILED"
    );

    if (!sessionResult)
    {
        return false;
    }

    _ready = true;

    Serial.print("Session: ");
    Serial.println(
        _sessionId
    );

    Serial.print("Session file: ");
    Serial.println(
        _logger.sessionPath()
    );

    _logger.logEvent(
        millis(),
        "PROTOTYPE_BOX_READY",
        "{\"memory\":\"KSJ_Storage-v0.2.0\"}"
    );

    if (hasPreviousSession)
    {
        const String payload =
            buildInspectionPayload(
                previousSessionId,
                previousInspection
            );

        _logger.logEvent(
            millis(),
            "PREVIOUS_SESSION_INSPECTED",
            payload.c_str()
        );
    }

    const String healthPayload =
        buildHealthPayload();

    _logger.logEvent(
        millis(),
        "STORAGE_HEALTH",
        healthPayload.c_str()
    );

    return true;
}

void StorageService::update(
    uint32_t nowMs,
    const KSJ::LightReading& light
)
{
    if (!_ready)
    {
        return;
    }

    _logger.update(
        nowMs
    );

    if (
        nowMs -
            _previousTelemetryMs <
        StorageServiceConfig::
            TELEMETRY_INTERVAL_MS
    )
    {
        return;
    }

    _previousTelemetryMs =
        nowMs;

    const String payload =
        buildLightPayload(
            light
        );

    const KSJ::StorageResult result =
        _logger.logTelemetry(
            nowMs,
            payload.c_str()
        );

    if (!result)
    {
        Serial.println(
            "Storage telemetry: FAILED"
        );

        _ready = false;
    }
}

KSJ::StorageResult StorageService::logEvent(
    uint32_t uptimeMs,
    const char* eventName,
    const char* payloadJson
)
{
    if (!_ready)
    {
        KSJ::StorageResult result;
        result.success = false;
        result.status =
            KSJ::StorageStatus::
                NotInitialized;

        return result;
    }

    return _logger.logEvent(
        uptimeMs,
        eventName,
        payloadJson
    );
}

bool StorageService::isReady() const
{
    return
        _ready &&
        _logger.isReady();
}

uint32_t StorageService::sessionNumber() const
{
    return _sessionNumber;
}

const char* StorageService::sessionId() const
{
    return _sessionId.c_str();
}

const char* StorageService::sessionPath() const
{
    return _logger.sessionPath();
}

const KSJ::StorageHealth&
StorageService::health() const
{
    return _health;
}

String StorageService::makeSessionId(
    uint32_t number
) const
{
    String formatted =
        String(number);

    while (formatted.length() < 6)
    {
        formatted =
            "0" +
            formatted;
    }

    return
        String(
            StorageServiceConfig::
                SESSION_PREFIX
        ) +
        formatted;
}

String StorageService::makeSessionPath(
    const String& sessionId
) const
{
    return
        "/session_" +
        sessionId +
        ".jsonl";
}

String StorageService::buildInspectionPayload(
    const String& previousSessionId,
    const KSJ::SessionInspection& inspection
) const
{
    String payload;

    payload.reserve(192);

    payload +=
        "{\"previous_session\":\"";

    payload +=
        previousSessionId;

    payload +=
        "\",\"result\":\"";

    payload +=
        KSJ::
            sessionInspectionStatusName(
                inspection.status
            );

    payload +=
        "\",\"size_bytes\":";

    payload +=
        String(
            static_cast<unsigned long>(
                inspection.sizeBytes
            )
        );

    payload +=
        "}";

    return payload;
}

String StorageService::buildHealthPayload() const
{
    String payload;

    payload.reserve(192);

    payload +=
        "{\"state\":\"";

    payload +=
        KSJ::storageHealthStateName(
            _health.state
        );

    payload +=
        "\",\"sessions\":";

    payload +=
        String(
            _health.sessionCount
        );

    payload +=
        ",\"total_bytes\":";

    payload +=
        String(
            static_cast<unsigned long>(
                _health.totalLogBytes
            )
        );

    payload +=
        ",\"incomplete\":";

    payload +=
        String(
            _health.incompleteSessionCount
        );

    payload +=
        "}";

    return payload;
}

String StorageService::buildLightPayload(
    const KSJ::LightReading& light
) const
{
    String payload;

    payload.reserve(160);

    payload +=
        "{\"ldr_raw\":";

    if (light.rawValid)
    {
        payload +=
            String(
                light.rawValue
            );
    }
    else
    {
        payload +=
            "null";
    }

    payload +=
        ",\"light_percent\":";

    if (light.lightPercentValid)
    {
        payload +=
            String(
                light.lightPercent,
                2
            );
    }
    else
    {
        payload +=
            "null";
    }

    payload +=
        ",\"measured_at_ms\":";

    payload +=
        String(
            light.measuredAtMs
        );

    payload +=
        ",\"free_heap\":";

    payload +=
        String(
            ESP.getFreeHeap()
        );

    payload +=
        "}";

    return payload;
}

void StorageService::printHealth() const
{
    Serial.println(
        "Storage health"
    );

    Serial.print("  State: ");

    Serial.println(
        KSJ::storageHealthStateName(
            _health.state
        )
    );

    Serial.print("  Sessions: ");

    Serial.println(
        _health.sessionCount
    );

    Serial.print("  Log bytes: ");

    Serial.println(
        static_cast<unsigned long>(
            _health.totalLogBytes
        )
    );

    Serial.print("  Incomplete: ");

    Serial.println(
        _health.incompleteSessionCount
    );
}