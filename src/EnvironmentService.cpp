#include "EnvironmentService.h"

EnvironmentService::EnvironmentService(
    TwoWire& wire
)
    : _aht20(
          wire,
          SensorConfig::AHT20_INTERVAL_MS
      ),
      _bmp280(
          SensorConfig::BMP280_INTERVAL_MS
      ),
      _lightConfig{
          SensorConfig::LDR_DARK_RAW,
          SensorConfig::LDR_BRIGHT_RAW,
          SensorConfig::
              LDR_HIGHER_RAW_IS_BRIGHTER,
          SensorConfig::LDR_INTERVAL_MS
      },
      _lightSensor(
          Pins::LDR,
          _lightConfig
      )
{
}

bool EnvironmentService::begin()
{
    _aht20Ready =
        _aht20.begin();

    _bmp280Ready =
        _bmp280.begin();

    _lightReady =
        _lightSensor.begin();

    Serial.print("AHT20: ");
    Serial.println(
        _aht20Ready
            ? "READY"
            : "NOT FOUND"
    );

    Serial.print("BMP280: ");
    Serial.println(
        _bmp280Ready
            ? "READY"
            : "NOT FOUND"
    );

    if (_bmp280Ready)
    {
        Serial.print(
            "BMP280 address: 0x"
        );

        Serial.println(
            _bmp280.detectedAddress(),
            HEX
        );
    }

    Serial.print("LDR: ");
    Serial.println(
        _lightReady
            ? "READY"
            : "INVALID CONFIG"
    );

    return isReady();
}

void EnvironmentService::update(
    uint32_t nowMs
)
{
    _aht20.update(
        nowMs
    );

    _bmp280.update(
        nowMs
    );

    _lightSensor.update(
        nowMs
    );

    if (_aht20.hasNewReading())
    {
        _climateChanged = true;
        _aht20.clearNewReadingFlag();
    }

    if (_bmp280.hasNewReading())
    {
        _pressureChanged = true;
        _bmp280.clearNewReadingFlag();
    }

    if (_lightSensor.hasNewReading())
    {
        _lightChanged = true;
        _lightSensor.clearNewReadingFlag();
    }
}

bool EnvironmentService::
consumeClimateChanged()
{
    const bool changed =
        _climateChanged;

    _climateChanged = false;

    return changed;
}

bool EnvironmentService::
consumePressureChanged()
{
    const bool changed =
        _pressureChanged;

    _pressureChanged = false;

    return changed;
}

bool EnvironmentService::
consumeLightChanged()
{
    const bool changed =
        _lightChanged;

    _lightChanged = false;

    return changed;
}

bool EnvironmentService::isReady() const
{
    return
        _aht20Ready &&
        _bmp280Ready &&
        _lightReady;
}

const KSJ::EnvironmentReading&
EnvironmentService::climate() const
{
    return _aht20.reading();
}

const KSJ::EnvironmentReading&
EnvironmentService::pressure() const
{
    return _bmp280.reading();
}

const KSJ::LightReading&
EnvironmentService::light() const
{
    return _lightSensor.reading();
}