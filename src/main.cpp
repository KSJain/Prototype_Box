#include <Arduino.h>
#include <Wire.h>
#include <cstring>

#include <KSJ_SSD1306Display.h>

#include <InputEvent.h>
#include <KSJ_RotaryEncoder.h>

#include <KSJ_AHT20Sensor.h>
#include <KSJ_BMP280Sensor.h>

#include <ControlAction.h>
#include <ControlDecision.h>
#include <KSJ_RangeController.h>
#include <RangeControllerConfig.h>

#include <KSJ_DigitalOutput.h>

#include "Menu.h"
#include "PrototypeBoxConfig.h"

enum class AppScreen : uint8_t
{
    Menu,
    Detail
};

namespace OutputPins
{
    constexpr uint8_t ONBOARD_LED = 2;
}

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

const KSJ::RangeControllerConfig humidityConfig
{
    75.0F,
    82.0F
};

KSJ::RangeController humidityController(
    humidityConfig
);

KSJ::DigitalOutput humidifierOutput(
    OutputPins::ONBOARD_LED,
    true
);

PrototypeBox::Menu menu;

AppScreen currentScreen =
    AppScreen::Menu;

bool environmentChanged = false;
bool humidityDecisionValid = false;
bool humidifierRequestedOn = false;

bool isEnvironmentSelected()
{
    return std::strcmp(
        menu.selectedItem(),
        "Environment"
    ) == 0;
}

const char* humidityControlText()
{
    if (!humidityDecisionValid)
    {
        return "ERROR";
    }

    return humidifierRequestedOn
        ? "HUMIDIFY"
        : "OFF";
}

void applyHumidityDecision(
    const KSJ::ControlDecision& decision
)
{
    humidityDecisionValid =
        decision.inputValid;

    switch (decision.action)
    {
        case KSJ::ControlAction::TurnOn:
        {
            humidifierRequestedOn = true;

            humidifierOutput.on();

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
            humidifierRequestedOn = false;

            humidifierOutput.off();

            Serial.print(
                "Humidity control: TURN OFF at "
            );

            if (decision.inputValid)
            {
                Serial.print(
                    decision.measuredValue,
                    1
                );

                Serial.println("%");
            }
            else
            {
                Serial.println(
                    "invalid sensor reading"
                );
            }

            Serial.println(
                "Onboard LED: OFF"
            );

            break;
        }

        case KSJ::ControlAction::Hold:
        default:
        {
            /*
             * Keep the previous recommendation
             * and physical output state.
             */
            break;
        }
    }
}

void updateHumidityControl()
{
    if (!aht20.hasNewReading())
    {
        return;
    }

    const KSJ::EnvironmentReading& reading =
        aht20.reading();

    const float humidity =
        reading.humidityValid
            ? reading.humidityPercent
            : NAN;

    const KSJ::ControlDecision decision =
        humidityController.update(
            humidity,
            reading.measuredAtMs
        );

    applyHumidityDecision(decision);
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
            16,
            "T:"
        );

        display.print(
            18,
            16,
            temperatureText.c_str()
        );

        display.print(
            54,
            16,
            "C"
        );
    }
    else
    {
        display.print(
            0,
            16,
            "T: --"
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
            28,
            "H:"
        );

        display.print(
            18,
            28,
            humidityText.c_str()
        );

        display.print(
            54,
            28,
            "%"
        );
    }
    else
    {
        display.print(
            0,
            28,
            "H: --"
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
            40,
            "P:"
        );

        display.print(
            18,
            40,
            pressureText.c_str()
        );

        display.print(
            72,
            40,
            "hPa"
        );
    }
    else
    {
        display.print(
            0,
            40,
            "P: --"
        );
    }

    display.print(
        0,
        52,
        "Control:"
    );

    display.print(
        54,
        52,
        humidityControlText()
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

    /*
     * Process the fresh AHT20 reading before
     * clearing its new-reading flag.
     */
    updateHumidityControl();

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

    /*
     * begin() configures the GPIO and immediately
     * places the output into the safe OFF state.
     */
    humidifierOutput.begin();

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

    Serial.println(
        "Humidity control:"
    );

    Serial.println(
        "  ON below 75%"
    );

    Serial.println(
        "  OFF above 82%"
    );

    Serial.println(
        "Onboard LED represents humidifier."
    );

    menu.draw(display);

    Serial.println("Display: ready");
    Serial.println("Input: ready");
    Serial.println("Environment: ready");
    Serial.println("Control: ready");
    Serial.println("Output: ready");
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