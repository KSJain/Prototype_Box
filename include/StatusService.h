#pragma once

#include <Arduino.h>
#include <KSJ_AddressableStrip.h>
#include <KSJ_Lighting.h>
#include <LedColor.h>

enum class StatusState : uint8_t
{
    Booting,
    Ready,
    Humidifying,
    Fault
};

class StatusService
{
public:
    StatusService(
        uint16_t pixelCount,
        uint8_t dataPin,
        uint8_t brightness
    );

    bool begin();
    void update();

    void setBooting();
    void setReady();
    void setHumidifying();
    void setFault();

    bool isReady() const;
    StatusState state() const;
    const char* statusText() const;

private:
    void setState(StatusState state);
    void applyLightingForState(StatusState state);
    const char* textForState(StatusState state) const;

    KSJ::AddressableStrip _strip;
    KSJ::Lighting _lighting;
    const uint8_t _brightness;

    StatusState _state = StatusState::Booting;
    bool _ready = false;
};
