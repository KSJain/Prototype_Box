#pragma once

#include <Arduino.h>

#include <ControlAction.h>
#include <EnvironmentReading.h>
#include <KSJ_DigitalOutput.h>
#include <KSJ_RangeController.h>
#include <RangeControllerConfig.h>

#include "config/SensorConfig.h"

class ControlService
{
public:
    ControlService(
        uint8_t outputPin,
        bool activeHigh
    );

    bool begin();

    KSJ::ControlAction update(
        const KSJ::EnvironmentReading& reading
    );

    bool isReady() const;

    bool isValid() const;

    bool isHumidifying() const;

    const char* statusText() const;

private:
    const KSJ::RangeControllerConfig
        _humidityConfig;

    KSJ::RangeController
        _humidityController;

    KSJ::DigitalOutput
        _humidifierOutput;

    bool _ready = false;
    bool _decisionValid = false;
    bool _humidifying = false;
};