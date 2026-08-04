#include "StatusService.h"

StatusService::StatusService(
    uint16_t pixelCount,
    uint8_t dataPin,
    uint8_t brightness
)
    : _strip(pixelCount, dataPin),
      _brightness(brightness)
{
}

bool StatusService::begin()
{
    _strip.setBrightness(_brightness);
    _ready = _strip.begin();

    Serial.print("Addressable strip: ");
    Serial.println(_ready ? "READY" : "FAILED");

    setBooting();

    return _ready;
}

void StatusService::setBooting()
{
    setState(StatusState::Booting);
}

void StatusService::setReady()
{
    setState(StatusState::Ready);
}

void StatusService::setHumidifying()
{
    setState(StatusState::Humidifying);
}

void StatusService::setFault()
{
    setState(StatusState::Fault);
}

bool StatusService::isReady() const
{
    return _ready;
}

StatusState StatusService::state() const
{
    return _state;
}

const char* StatusService::statusText() const
{
    return textForState(_state);
}

void StatusService::setState(
    StatusState state
)
{
    _state = state;

    if (!_ready)
    {
        return;
    }

    _strip.fill(colorForState(state));
    _strip.show();

    Serial.print("Status strip: ");
    Serial.println(textForState(state));
}

KSJ::LedColor StatusService::colorForState(
    StatusState state
) const
{
    switch (state)
    {
        case StatusState::Booting:
            return KSJ::LedColor::yellow();

        case StatusState::Ready:
            return KSJ::LedColor::greenColor();

        case StatusState::Humidifying:
            return KSJ::LedColor::blueColor();

        case StatusState::Fault:
            return KSJ::LedColor::redColor();

        default:
            return KSJ::LedColor::off();
    }
}

const char* StatusService::textForState(
    StatusState state
) const
{
    switch (state)
    {
        case StatusState::Booting:
            return "BOOTING";

        case StatusState::Ready:
            return "READY";

        case StatusState::Humidifying:
            return "HUMIDIFYING";

        case StatusState::Fault:
            return "FAULT";

        default:
            return "UNKNOWN";
    }
}