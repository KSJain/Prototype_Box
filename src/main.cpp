#include <Arduino.h>
#include <Wire.h>
#include <cstring>

#include <KSJ_SSD1306Display.h>

#include <InputEvent.h>
#include <KSJ_RotaryEncoder.h>

#include <KSJ_AHT20Sensor.h>
#include <KSJ_BMP280Sensor.h>

#include "Menu.h"
#include "PrototypeBoxConfig.h"

enum class AppScreen : uint8_t
{
    Menu,
    Detail
};

KSJ::SSD1306Display display(
    DisplayConfig::WIDTH,
    DisplayConfig::HEIGHT,
    DisplayConfig::I2C_ADDRESS
);

KSJ::RotaryEncoder encoder(
    Pins::ROTARY_CLK,
    Pins::ROTARY_DT,
    Pins::ROTARY_SW
);

KSJ::AHT20Sensor aht20(
    Wire,
    2000
);

KSJ::BMP280Sensor bmp280(
    1000
);

PrototypeBox::Menu menu;

AppScreen currentScreen = AppScreen::Menu;

bool environmentChanged = false;

bool isEnvironmentSelected()
{
    return std::strcmp(
        menu.selectedItem(),
        "Environment"
    ) == 0;
}

void drawEnvironmentScreen()
{
    const KSJ::EnvironmentReading& ahtReading =
        aht20.reading();

    const KSJ::EnvironmentReading& bmpReading =
        bmp280.reading();

    display.clear();

    display.setTextSize(1);
    display.print(
        0,
        0,
        "RAMU Environment"
    );

    display.drawLine(
        0,
        10,
        127,
        10
    );

    if (ahtReading.temperatureValid)
    {
        const String temperatureText =
            String(
                ahtReading.temperatureC,
                1
            );

        display.print(
            0,
            18,
            "Temp:"
        );

        display.print(
            42,
            18,
            temperatureText.c_str()
        );

        display.print(
            78,
            18,
            "C"
        );
    }
    else
    {
        display.print(
            0,
            18,
            "Temp: --"
        );
    }

    if (ahtReading.humidityValid)
    {
        const String humidityText =
            String(
                ahtReading.humidityPercent,
                1
            );

        display.print(
            0,
            32,
            "Humidity:"
        );

        display.print(
            60,
            32,
            humidityText.c_str()
        );

        display.print(
            96,
            32,
            "%"
        );
    }
    else
    {
        display.print(
            0,
            32,
            "Humidity: --"
        );
    }

    if (bmpReading.pressureValid)
    {
        const String pressureText =
            String(
                bmpReading.pressureHpa,
                1
            );

        display.print(
            0,
            46,
            "Pressure:"
        );

        display.print(
            54,
            46,
            pressureText.c_str()
        );
    }
    else
    {
        display.print(
            0,
            46,
            "Pressure: --"
        );
    }

    display.print(
        0,
        56,
        "Press to return"
    );

    display.update();
}

void drawGenericDetailScreen()
{
    display.clear();

    display.setTextSize(1);
    display.print(
        0,
        0,
        "Selected:"
    );

    display.drawLine(
        0,
        10,
        127,
        10
    );

    display.setTextSize(2);
    display.print(
        0,
        20,
        menu.selectedItem()
    );

    display.setTextSize(1);
    display.print(
        0,
        52,
        "Press to return"
    );

    display.update();
}

void drawDetailScreen()
{
    if (isEnvironmentSelected())
    {
        drawEnvironmentScreen();
        return;
    }

    drawGenericDetailScreen();
}

void updateEnvironmentSensors()
{
    const uint32_t nowMs =
        millis();

    aht20.update(nowMs);
    bmp280.update(nowMs);

    if (aht20.hasNewReading())
    {
        environmentChanged = true;
        aht20.clearNewReadingFlag();
    }

    if (bmp280.hasNewReading())
    {
        environmentChanged = true;
        bmp280.clearNewReadingFlag();
    }
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    Serial.println();
    Serial.println(Board::NAME);

    Serial.print("Version: ");
    Serial.println(Board::VERSION);

    Serial.print("MCU: ");
    Serial.println(Board::MCU);

    Wire.begin(
        Pins::SDA,
        Pins::SCL
    );

    if (!display.begin())
    {
        Serial.println(
            "Display initialization failed."
        );

        while (true)
        {
            delay(1000);
        }
    }

    encoder.begin();

    const bool aht20Ready =
        aht20.begin();

    const bool bmp280Ready =
        bmp280.begin();

    Serial.print("AHT20: ");
    Serial.println(
        aht20Ready
            ? "READY"
            : "NOT FOUND"
    );

    Serial.print("BMP280: ");
    Serial.println(
        bmp280Ready
            ? "READY"
            : "NOT FOUND"
    );

    if (bmp280Ready)
    {
        Serial.print(
            "BMP280 address: 0x"
        );

        Serial.println(
            bmp280.detectedAddress(),
            HEX
        );
    }

    menu.draw(display);

    Serial.println("Display: ready");
    Serial.println("Input: ready");
    Serial.println("Environment: ready");
    Serial.println("Prototype Box: ready");
}

void loop()
{
    updateEnvironmentSensors();

    if (
        currentScreen == AppScreen::Detail &&
        isEnvironmentSelected() &&
        environmentChanged
    )
    {
        drawEnvironmentScreen();
        environmentChanged = false;
    }

    const KSJ::InputEvent event =
        encoder.update();

    if (event == KSJ::InputEvent::None)
    {
        return;
    }

    switch (currentScreen)
    {
        case AppScreen::Menu:
        {
            menu.update(event);

            switch (event)
            {
                case KSJ::InputEvent::RotateLeft:
                case KSJ::InputEvent::RotateRight:
                {
                    Serial.print("Menu: ");
                    Serial.println(
                        menu.selectedItem()
                    );

                    menu.draw(display);
                    break;
                }

                case KSJ::InputEvent::ButtonDown:
                {
                    if (menu.consumeSelection())
                    {
                        currentScreen =
                            AppScreen::Detail;

                        Serial.print("Opened: ");
                        Serial.println(
                            menu.selectedItem()
                        );

                        drawDetailScreen();
                        environmentChanged = false;
                    }

                    break;
                }

                case KSJ::InputEvent::ButtonUp:
                case KSJ::InputEvent::None:
                default:
                    break;
            }

            break;
        }

        case AppScreen::Detail:
        {
            if (event == KSJ::InputEvent::ButtonDown)
            {
                currentScreen =
                    AppScreen::Menu;

                Serial.println(
                    "Returned to menu"
                );

                menu.draw(display);
                environmentChanged = false;
            }

            break;
        }
    }
}