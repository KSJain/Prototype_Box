#pragma once

#include <Arduino.h>
#include <SPI.h>

#include <KSJ_SDStorage.h>
#include <RetentionPolicy.h>
#include <SessionInspection.h>
#include <SessionInspector.h>
#include <SessionLogger.h>
#include <SessionRetention.h>
#include <SessionSequence.h>
#include <StorageHealth.h>
#include <StorageResult.h>

#include <LightReading.h>

class StorageService
{
public:
    StorageService(
        SPIClass& spi,
        KSJ::SDStorage& storage
    );

    bool begin(
        const char* firmwareVersion,
        const char* boardName
    );

    void update(
        uint32_t nowMs,
        const KSJ::LightReading& light
    );

    KSJ::StorageResult logEvent(
        uint32_t uptimeMs,
        const char* eventName,
        const char* payloadJson = nullptr
    );

    bool isReady() const;

    uint32_t sessionNumber() const;

    const char* sessionId() const;

    const char* sessionPath() const;

    const KSJ::StorageHealth& health() const;

    KSJ::StorageResult savePeerBinding(
        uint32_t uptimeMs,
        const char* hostId,
        const char* peerId,
        const char* peerFingerprint,
        const char* bindingId,
        const char* capabilities,
        const char* reason = "BOUND"
    );

    KSJ::StorageResult recordPeerUnbound(
        uint32_t uptimeMs,
        const char* hostId,
        const char* peerId,
        const char* peerFingerprint,
        const char* bindingId,
        const char* capabilities
    );

private:
    String makeSessionId(
        uint32_t number
    ) const;

    String makeSessionPath(
        const String& sessionId
    ) const;

    String buildInspectionPayload(
        const String& previousSessionId,
        const KSJ::SessionInspection& inspection
    ) const;

    String buildHealthPayload() const;

    String buildLightPayload(
        const KSJ::LightReading& light
    ) const;

    void printHealth() const;

    SPIClass& _spi;
    KSJ::SDStorage& _storage;

    KSJ::SessionLogger _logger;
    KSJ::SessionSequence _sequence;
    KSJ::SessionInspector _inspector;
    KSJ::SessionRetention _retention;

    KSJ::StorageHealth _health;

    String _sessionId;

    uint32_t _sessionNumber = 0;
    uint32_t _previousTelemetryMs = 0;

    bool _ready = false;
};