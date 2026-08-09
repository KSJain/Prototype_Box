#include "StatusService.h"

StatusService::StatusService(
    uint16_t pixelCount,
    uint8_t dataPin,
    uint8_t brightness
)
    : _strip(pixelCount, dataPin),
      _lighting(_strip),
      _brightness(brightness)
{
}

bool StatusService::begin()
{
    _ready = _lighting.begin(_brightness);

    Serial.print("Addressable strip: ");
    Serial.println(_ready ? "READY" : "FAILED");

    setBooting();

    return _ready;
}

void StatusService::update()
{
    if (!_ready)
    {
        return;
    }

    _lighting.tick();
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

    applyLightingForState(state);

    // Render the new state immediately. Animated states continue
    // advancing through update() from the application loop.
    _lighting.tick();

    Serial.print("Status lighting: ");
    Serial.println(textForState(state));
}

void StatusService::applyLightingForState(
    StatusState state
)
{
    switch (state)
    {
        case StatusState::Booting:
            // setup() is intentionally not an animation loop, so booting
            // remains a dependable static yellow indicator.
            _lighting.setColors(
                KSJ::LedColor::yellow(),
                KSJ::LedColor::yellow()
            );
            _lighting.setArt(
                KSJ::LightArt::Solid
            );
            break;

        case StatusState::Ready:
            // A quiet green heartbeat proves KSJ_Lighting continues to tick
            // without blocking the rest of Prototype Box.
            _lighting.setColors(
                KSJ::LedColor::greenColor(),
                KSJ::LedColor::greenColor()
            );
            _lighting.setArt(
                KSJ::LightArt::Breathe
            );
            break;

        case StatusState::Humidifying:
            _lighting.setColors(
                KSJ::LedColor::blueColor(),
                KSJ::LedColor(0, 180, 255)
            );
            _lighting.setArt(
                KSJ::LightArt::Tide
            );
            break;

        case StatusState::Fault:
            // Fault remains deliberately static and unambiguous.
            // It must not depend on animation timing to remain visible.
            _lighting.setColors(
                KSJ::LedColor::redColor(),
                KSJ::LedColor::redColor()
            );
            _lighting.setArt(
                KSJ::LightArt::Solid
            );
            break;
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
