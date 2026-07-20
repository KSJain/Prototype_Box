#pragma once

#include <Arduino.h>

namespace Board
{
    constexpr char NAME[] = "Prototype Box v1";
    constexpr char VERSION[] = "0.1.0";
    constexpr char MCU[] = "ESP32 DevKit V1";
}

namespace Pins
{
    constexpr uint8_t SDA = 21;
    constexpr uint8_t SCL = 22;

    constexpr uint8_t ROTARY_CLK = 27;
    constexpr uint8_t ROTARY_DT = 26;
    constexpr uint8_t ROTARY_SW = 25;
}

namespace DisplayConfig
{
    constexpr uint8_t WIDTH = 128;
    constexpr uint8_t HEIGHT = 64;
    constexpr uint8_t I2C_ADDRESS = 0x3C;
}

namespace Features
{
    constexpr bool HAS_DISPLAY = true;
    constexpr bool HAS_ROTARY = true;
}