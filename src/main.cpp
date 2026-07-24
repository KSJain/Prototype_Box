#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
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

#include <KSJ_SDStorage.h>
#include <StorageInfo.h>
#include <StorageResult.h>
#include <StorageStatus.h>

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

namespace StoragePins
{
    constexpr uint8_t CS = 5;
    constexpr uint8_t SCK = 18;
    constexpr uint8_t MISO = 19;
    constexpr uint8_t MOSI = 23;
}

namespace StorageConfig
{
    constexpr uint32_t SPI_FREQUENCY_HZ =
        4'000'000;

    constexpr char BOOT_LOG_PATH[] =
        "/boot_log.txt";
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

SPIClass sdSpi(VSPI);

KSJ::SDStorage storage(
    sdSpi,
    StoragePins::CS,
    StorageConfig::SPI_FREQUENCY_HZ
);

PrototypeBox::Menu menu;

AppScreen currentScreen =
    AppScreen::Menu;

bool environmentChanged = false;
bool humidityDecisionValid = false;
bool humidifierRequestedOn = false;

bool storageReady = false;
uint32_t bootCount = 0;

bool isEnvironmentSelected()
{
    return std::strcmp(
        menu.selectedItem(),
        "Environment"
    ) == 0;
}

const char* storageStatusName(
    KSJ::StorageStatus status
)
{
    switch (status)
    {
        case KSJ::StorageStatus::NotInitialized:
            return "NOT INITIALIZED";

        case KSJ::StorageStatus::Ready:
            return "READY";

        case KSJ::StorageStatus::MountFailed:
            return "MOUNT FAILED";

        case KSJ::StorageStatus::CardNotFound:
            return "CARD NOT FOUND";

        case KSJ::StorageStatus::OpenFailed:
            return "OPEN FAILED";

        case KSJ::StorageStatus::WriteFailed:
            return "WRITE FAILED";

        case KSJ::StorageStatus::ReadFailed:
            return "READ FAILED";

        default:
            return "UNKNOWN";
    }
}

const char* cardTypeName(
    KSJ::StorageCardType cardType
)
{
    switch (cardType)
    {
        case KSJ::StorageCardType::MMC:
            return "MMC";

        case KSJ::StorageCardType::SDSC:
            return "SDSC";

        case KSJ::StorageCardType::SDHC:
            return "SDHC/SDXC";

        case KSJ::StorageCardType::None:
            return "NONE";

        case KSJ::StorageCardType::Unknown:
        default:
            return "UNKNOWN";
    }
}

uint32_t countLines(
    const String& text
)
{
    if (text.length() == 0)
    {
        return 0;
    }

    uint32_t lineCount = 0;

    for (
        size_t index = 0;
        index < text.length();
        ++index
    )
    {
        if (text[index] == '\n')
        {
            ++lineCount;
        }
    }

    /*
     * Count a final line even if the file does
     * not end with a newline character.
     */
    if (text[text.length() - 1] != '\n')
    {
        ++lineCount;
    }

    return lineCount;
}

void printStorageInfo()
{
    const KSJ::StorageInfo info =
        storage.info();

    if (!info.valid)
    {
        Serial.println(
            "Storage information unavailable."
        );

        return;
    }

    constexpr uint64_t BYTES_PER_MB =
        1024ULL * 1024ULL;

    Serial.print("Card type: ");
    Serial.println(
        cardTypeName(info.cardType)
    );

    Serial.print("Card size: ");
    Serial.print(
        info.cardSizeBytes /
        BYTES_PER_MB
    );
    Serial.println(" MB");

    Serial.print("Filesystem size: ");
    Serial.print(
        info.totalBytes /
        BYTES_PER_MB
    );
    Serial.println(" MB");

    Serial.print("Used space: ");
    Serial.print(
        info.usedBytes /
        BYTES_PER_MB
    );
    Serial.println(" MB");
}

void recordBoot()
{
    if (!storageReady)
    {
        return;
    }

    String existingBootLog;

    const KSJ::StorageResult readResult =
        storage.readText(
            StorageConfig::BOOT_LOG_PATH,
            existingBootLog
        );

    if (readResult.success)
    {
        bootCount =
            countLines(existingBootLog);
    }
    else
    {
        /*
         * An absent boot log is expected on the
         * first run, so begin at zero.
         */
        bootCount = 0;
    }

    ++bootCount;

    String bootRecord;

    bootRecord += "boot=";
    bootRecord += String(bootCount);

    bootRecord += ",firmware=";
    bootRecord += Board::VERSION;

    bootRecord += ",uptime_ms=";
    bootRecord += String(millis());

    bootRecord += "\n";

    KSJ::StorageResult writeResult;

    if (bootCount == 1)
    {
        writeResult =
            storage.writeText(
                StorageConfig::BOOT_LOG_PATH,
                bootRecord.c_str()
            );
    }
    else
    {
        writeResult =
            storage.appendText(
                StorageConfig::BOOT_LOG_PATH,
                bootRecord.c_str()
            );
    }

    if (!writeResult.success)
    {
        Serial.print(
            "Boot record failed: "
        );

        Serial.println(
            storageStatusName(
                writeResult.status
            )
        );

        return;
    }

    Serial.print("Boot record saved. Boot #");
    Serial.println(bootCount);
}

void initializeStorage()
{
    sdSpi.begin(
        StoragePins::SCK,
        StoragePins::MISO,
        StoragePins::MOSI,
        StoragePins::CS
    );

    const KSJ::StorageResult mountResult =
        storage.begin();

    storageReady =
        mountResult.success;

    Serial.print("Storage: ");

    Serial.println(
        storageReady
            ? "READY"
            : storageStatusName(
                mountResult.status
            )
    );

    if (!storageReady)
    {
        Serial.println(
            "Prototype Box will continue without persistence."
        );

        return;
    }

    printStorageInfo();
    recordBoot();
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
            Serial.println("Onboard LED: ON");

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

            Serial.println("Onboard LED: OFF");

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

        display.print(0, 16, "T:");

        display.print(
            18,
            16,
            temperatureText.c_str()
        );

        display.print(54, 16, "C");
    }
    else
    {
        display.print(0, 16, "T: --");
    }

    if (ahtReading.humidityValid)
    {
        const String humidityText =
            String(
                ahtReading.humidityPercent,
                1
            );

        display.print(0, 28, "H:");

        display.print(
            18,
            28,
            humidityText.c_str()
        );

        display.print(54, 28, "%");
    }
    else
    {
        display.print(0, 28, "H: --");
    }

    if (bmpReading.pressureValid)
    {
        const String pressureText =
            String(
                bmpReading.pressureHpa,
                1
            );

        display.print(0, 40, "P:");

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
        display.print(0, 40, "P: --");
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
    delay(500);

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

    /*
     * Storage failure is non-fatal.
     * Environmental control continues locally.
     */
    initializeStorage();

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

    Serial.print("Storage: ");
    Serial.println(
        storageReady
            ? "ready"
            : "unavailable"
    );

    Serial.print("Persistent boot count: ");
    Serial.println(bootCount);

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