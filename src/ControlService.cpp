#include "ControlService.h"

ControlService::ControlService(
    uint8_t outputPin,
    bool activeHigh
)
    : _humidityConfig{
          SensorConfig::
              HUMIDITY_ON_BELOW_PERCENT,
          SensorConfig::
              HUMIDITY_OFF_ABOVE_PERCENT
      },
      _humidityController(
          _humidityConfig
      ),
      _humidifierOutput(
          outputPin,
          activeHigh
      )
{
}

bool ControlService::begin()
{
    _humidifierOutput.begin();

    _humidifierOutput.off();

    _humidifying = false;
    _decisionValid = false;
    _ready = true;

    return true;
}

KSJ::ControlAction ControlService::update(
    const KSJ::EnvironmentReading& reading
)
{
    if (!_ready)
    {
        _decisionValid = false;

        return KSJ::ControlAction::Hold;
    }

    const float humidity =
        reading.humidityValid
            ? reading.humidityPercent
            : NAN;

    const KSJ::ControlDecision decision =
        _humidityController.update(
            humidity,
            reading.measuredAtMs
        );

    _decisionValid =
        decision.inputValid;

    if (!decision.inputValid)
    {
        Serial.println(
            "Humidity control: INVALID"
        );

        return decision.action;
    }

    switch (decision.action)
    {
        case KSJ::ControlAction::TurnOn:
        {
            _humidifierOutput.on();
            _humidifying = true;

            Serial.print(
                "Humidity control: TURN ON at "
            );

            Serial.print(
                decision.measuredValue,
                1
            );

            Serial.println("%");

            Serial.println(
                "Onboard LED: ON"
            );

            break;
        }

        case KSJ::ControlAction::TurnOff:
        {
            _humidifierOutput.off();
            _humidifying = false;

            Serial.print(
                "Humidity control: TURN OFF at "
            );

            Serial.print(
                decision.measuredValue,
                1
            );

            Serial.println("%");

            Serial.println(
                "Onboard LED: OFF"
            );

            break;
        }

        case KSJ::ControlAction::Hold:
        default:
            break;
    }

    return decision.action;
}

bool ControlService::isReady() const
{
    return _ready;
}

bool ControlService::isValid() const
{
    return
        _ready &&
        _decisionValid;
}

bool ControlService::isHumidifying() const
{
    return
        _ready &&
        _humidifying;
}

const char* ControlService::statusText() const
{
    if (!_ready)
    {
        return "UNAVAILABLE";
    }

    if (!_decisionValid)
    {
        return "ERROR";
    }

    return
        _humidifying
            ? "ON"
            : "OFF";
}