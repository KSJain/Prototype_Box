#pragma once

#include <Arduino.h>
#include <Wire.h>

#include <EnvironmentReading.h>
#include <KSJ_AHT20Sensor.h>
#include <KSJ_AnalogLightSensor.h>
#include <KSJ_BMP280Sensor.h>
#include <LightReading.h>

#include "config/PinConfig.h"
#include "config/SensorConfig.h"

class EnvironmentService
{
public:
    explicit EnvironmentService(
        TwoWire& wire
    );

    bool begin();

    void update(
        uint32_t nowMs
    );

    bool consumeClimateChanged();

    bool consumePressureChanged();

    bool consumeLightChanged();

    bool isReady() const;

    const KSJ::EnvironmentReading&
    climate() const;

    const KSJ::EnvironmentReading&
    pressure() const;

    const KSJ::LightReading&
    light() const;

private:
    KSJ::AHT20Sensor _aht20;
    KSJ::BMP280Sensor _bmp280;

    const KSJ::AnalogLightConfig
        _lightConfig;

    KSJ::AnalogLightSensor
        _lightSensor;

    bool _aht20Ready = false;
    bool _bmp280Ready = false;
    bool _lightReady = false;

    bool _climateChanged = false;
    bool _pressureChanged = false;
    bool _lightChanged = false;
};