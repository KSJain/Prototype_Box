#include <Arduino.h>
#include <Wire.h>

#include <KSJ_SSD1306Display.h>
#include <KSJ_RotaryEncoder.h>
#include <InputEvent.h>

#include "Menu.h"
#include "PrototypeBoxConfig.h"

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

PrototypeBox::Menu menu;

void showSelectedItem(const char* item)
{
    display.clear();

    display.setTextSize(1);
    display.print(0, 0, "Selected:");

    display.setTextSize(2);
    display.print(0, 20, item);

    display.setTextSize(1);
    display.print(0, 52, "Release to return");

    display.update();
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
        Serial.println("Display initialization failed.");

        while (true)
        {
            delay(1000);
        }
    }

    encoder.begin();

    menu.draw(display);

    Serial.println("Display: ready");
    Serial.println("Input: ready");
    Serial.println("Prototype Box: ready");
}

void loop()
{
    const KSJ::InputEvent event =
        encoder.update();

    if (event == KSJ::InputEvent::None)
    {
        return;
    }

    menu.update(event);

    switch (event)
    {
        case KSJ::InputEvent::RotateLeft:
        {
            Serial.print("Menu: ");
            Serial.println(menu.selectedItem());

            menu.draw(display);
            break;
        }

        case KSJ::InputEvent::RotateRight:
        {
            Serial.print("Menu: ");
            Serial.println(menu.selectedItem());

            menu.draw(display);
            break;
        }

        case KSJ::InputEvent::ButtonDown:
        {
            if (menu.consumeSelection())
            {
                Serial.print("Selected: ");
                Serial.println(menu.selectedItem());

                showSelectedItem(
                    menu.selectedItem()
                );
            }

            break;
        }

        case KSJ::InputEvent::ButtonUp:
        {
            menu.draw(display);
            break;
        }

        case KSJ::InputEvent::None:
        default:
        {
            break;
        }
    }
}