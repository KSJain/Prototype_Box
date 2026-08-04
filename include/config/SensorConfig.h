#pragma once

#include <Arduino.h>

namespace SensorConfig
{
    constexpr uint32_t AHT20_INTERVAL_MS =
        2000;

    constexpr uint32_t BMP280_INTERVAL_MS =
        1000;

    constexpr uint16_t LDR_DARK_RAW =
        650;

    constexpr uint16_t LDR_BRIGHT_RAW =
        3650;

    constexpr bool LDR_HIGHER_RAW_IS_BRIGHTER =
        true;

    constexpr uint32_t LDR_INTERVAL_MS =
        500;

    constexpr float HUMIDITY_ON_BELOW_PERCENT =
        75.0F;

    constexpr float HUMIDITY_OFF_ABOVE_PERCENT =
        82.0F;
}