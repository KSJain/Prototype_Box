#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <cstring>

#include <KSJ_SSD1306Display.h>

#include <InputEvent.h>
#include <KSJ_RotaryEncoder.h>

#include <KSJ_AHT20Sensor.h>
#include <KSJ_BMP280Sensor.h>
#include <KSJ_AnalogLightSensor.h>
#include <AnalogLightConfig.h>
#include <LightReading.h>

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

enum class EnvironmentPage : uint8_t
{
    Climate,
    Light
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
        4000000;

    constexpr char BOOT_LOG_PATH[] =
        "/boot_log.txt";
}

namespace LightCalibration
{
    constexpr uint16_t DARK_RAW = 650;
    constexpr uint16_t BRIGHT_RAW = 3650;
    constexpr bool HIGHER_RAW_IS_BRIGHTER = true;
    constexpr uint32_t REFRESH_INTERVAL_MS = 500;
}

namespace SerialUI
{
    constexpr uint32_t LIVE_REFRESH_INTERVAL_MS =
        2000;
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

const KSJ::AnalogLightConfig lightConfig
{
    LightCalibration::DARK_RAW,
    LightCalibration::BRIGHT_RAW,
    LightCalibration::HIGHER_RAW_IS_BRIGHTER,
    LightCalibration::REFRESH_INTERVAL_MS
};

KSJ::AnalogLightSensor lightSensor(
    Pins::LDR,
    lightConfig
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

EnvironmentPage environmentPage =
    EnvironmentPage::Climate;

bool environmentChanged = false;
bool humidityDecisionValid = false;
bool humidifierRequestedOn = false;

bool storageReady = false;
uint32_t bootCount = 0;

uint32_t previousSerialPageMs = 0;
bool serialPageRefreshRequested = false;

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

    bootCount =
        readResult.success
            ? countLines(existingBootLog)
            : 0;

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
        ? "ON"
        : "OFF";
}

const char* lightConditionText(
    const KSJ::LightReading& reading
)
{
    if (!reading.lightPercentValid)
    {
        return "Unavailable";
    }

    const float percent =
        reading.lightPercent;

    if (percent < 5.0F)
    {
        return "Dark";
    }

    if (percent < 20.0F)
    {
        return "Dim";
    }

    if (percent < 45.0F)
    {
        return "Shady";
    }

    if (percent < 75.0F)
    {
        return "Bright";
    }

    return "Very bright";
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

void printClimatePageToSerial()
{
    const KSJ::EnvironmentReading& ahtReading =
        aht20.reading();

    const KSJ::EnvironmentReading& bmpReading =
        bmp280.reading();

    Serial.println();
    Serial.println(
        "Environment > Climate"
    );
    Serial.println(
        "------------------------"
    );

    Serial.print("Temperature: ");

    if (ahtReading.temperatureValid)
    {
        Serial.print(
            ahtReading.temperatureC,
            1
        );
        Serial.println(" C");
    }
    else
    {
        Serial.println("INVALID");
    }

    Serial.print("Humidity:    ");

    if (ahtReading.humidityValid)
    {
        Serial.print(
            ahtReading.humidityPercent,
            1
        );
        Serial.println(" %");
    }
    else
    {
        Serial.println("INVALID");
    }

    Serial.print("Pressure:    ");

    if (bmpReading.pressureValid)
    {
        Serial.print(
            bmpReading.pressureHpa,
            1
        );
        Serial.println(" hPa");
    }
    else
    {
        Serial.println("INVALID");
    }

    Serial.print("Control:     ");
    Serial.println(
        humidityControlText()
    );

    Serial.println(
        "Rotate = Page | Press = Menu"
    );
    Serial.println(
        "------------------------"
    );
}

void printLightPageToSerial()
{
    const KSJ::LightReading& reading =
        lightSensor.reading();

    Serial.println();
    Serial.println(
        "Environment > Light"
    );
    Serial.println(
        "------------------------"
    );

    Serial.print("Raw ADC:     ");

    if (reading.rawValid)
    {
        Serial.println(
            reading.rawValue
        );
    }
    else
    {
        Serial.println("INVALID");
    }

    Serial.print("Light:       ");

    if (reading.lightPercentValid)
    {
        Serial.print(
            reading.lightPercent,
            1
        );
        Serial.println(" %");
    }
    else
    {
        Serial.println("INVALID");
    }

    Serial.print("Condition:   ");
    Serial.println(
        lightConditionText(reading)
    );

    Serial.print("Measured at: ");
    Serial.print(
        reading.measuredAtMs
    );
    Serial.println(" ms");

    Serial.println(
        "Rotate = Page | Press = Menu"
    );
    Serial.println(
        "------------------------"
    );
}

void printActiveEnvironmentPageToSerial()
{
    if (
        currentScreen != AppScreen::Detail ||
        !isEnvironmentSelected()
    )
    {
        return;
    }

    switch (environmentPage)
    {
        case EnvironmentPage::Climate:
            printClimatePageToSerial();
            break;

        case EnvironmentPage::Light:
            printLightPageToSerial();
            break;
    }
}

void updateSerialInterface(
    uint32_t nowMs
)
{
    if (
        currentScreen != AppScreen::Detail ||
        !isEnvironmentSelected()
    )
    {
        serialPageRefreshRequested = false;
        return;
    }

    const bool intervalElapsed =
        nowMs - previousSerialPageMs >=
        SerialUI::LIVE_REFRESH_INTERVAL_MS;

    if (
        !serialPageRefreshRequested &&
        !intervalElapsed
    )
    {
        return;
    }

    previousSerialPageMs = nowMs;
    serialPageRefreshRequested = false;

    printActiveEnvironmentPageToSerial();
}

void drawClimatePage()
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
        "Environment 1/2 >"
    );

    display.drawLine(
        0,
        10,
        127,
        10
    );

    if (ahtReading.temperatureValid)
    {
        const String value =
            String(
                ahtReading.temperatureC,
                1
            );

        display.print(0, 15, "Temp");
        display.print(
            42,
            15,
            value.c_str()
        );
        display.print(78, 15, "C");
    }
    else
    {
        display.print(
            0,
            15,
            "Temp      --"
        );
    }

    if (ahtReading.humidityValid)
    {
        const String value =
            String(
                ahtReading.humidityPercent,
                1
            );

        display.print(0, 27, "Humidity");
        display.print(
            54,
            27,
            value.c_str()
        );
        display.print(90, 27, "%");
    }
    else
    {
        display.print(
            0,
            27,
            "Humidity  --"
        );
    }

    if (bmpReading.pressureValid)
    {
        const String value =
            String(
                bmpReading.pressureHpa,
                0
            );

        display.print(0, 39, "Pressure");
        display.print(
            54,
            39,
            value.c_str()
        );
        display.print(
            84,
            39,
            "hPa"
        );
    }
    else
    {
        display.print(
            0,
            39,
            "Pressure  --"
        );
    }

    display.print(0, 51, "Control");
    display.print(
        48,
        51,
        humidityControlText()
    );
    display.print(
        78,
        51,
        "Btn=Menu"
    );

    display.update();
}

void drawLightPage()
{
    const KSJ::LightReading& reading =
        lightSensor.reading();

    display.clear();
    display.setTextSize(1);

    display.print(
        0,
        0,
        "< Environment 2/2"
    );

    display.drawLine(
        0,
        10,
        127,
        10
    );

    display.print(0, 16, "Light:");

    if (reading.lightPercentValid)
    {
        const String value =
            String(
                reading.lightPercent,
                1
            );

        display.print(
            42,
            16,
            value.c_str()
        );
        display.print(78, 16, "%");
    }
    else
    {
        display.print(42, 16, "--");
    }

    display.print(0, 30, "Mood:");
    display.print(
        36,
        30,
        lightConditionText(reading)
    );

    display.print(0, 44, "Raw:");

    if (reading.rawValid)
    {
        const String value =
            String(
                reading.rawValue
            );

        display.print(
            30,
            44,
            value.c_str()
        );
    }
    else
    {
        display.print(30, 44, "--");
    }

    display.print(
        0,
        56,
        "Rotate | Btn=Menu"
    );

    display.update();
}

void drawEnvironmentScreen()
{
    switch (environmentPage)
    {
        case EnvironmentPage::Climate:
            drawClimatePage();
            break;

        case EnvironmentPage::Light:
            drawLightPage();
            break;
    }
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

void changeEnvironmentPage(
    KSJ::InputEvent event
)
{
    if (
        event != KSJ::InputEvent::RotateLeft &&
        event != KSJ::InputEvent::RotateRight
    )
    {
        return;
    }

    environmentPage =
        environmentPage ==
            EnvironmentPage::Climate
        ? EnvironmentPage::Light
        : EnvironmentPage::Climate;

    drawEnvironmentScreen();
    serialPageRefreshRequested = true;
}

void updateEnvironmentSensors(
    uint32_t nowMs
)
{
    aht20.update(nowMs);
    bmp280.update(nowMs);
    lightSensor.update(nowMs);

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

    if (lightSensor.hasNewReading())
    {
        environmentChanged = true;
        lightSensor.clearNewReadingFlag();
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

    Wire.setClock(100000);

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
    humidifierOutput.begin();

    const bool aht20Ready =
        aht20.begin();

    const bool bmp280Ready =
        bmp280.begin();

    const bool lightReady =
        lightSensor.begin();

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

    Serial.print("LDR: ");
    Serial.println(
        lightReady
            ? "READY"
            : "INVALID CONFIG"
    );

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

    menu.draw(display);

    Serial.println("Display: ready");
    Serial.println("Input: ready");
    Serial.println("Environment: ready");
    Serial.println("Light: ready");
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
    const uint32_t nowMs =
        millis();

    updateEnvironmentSensors(
        nowMs
    );

    updateSerialInterface(
        nowMs
    );

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

                        if (isEnvironmentSelected())
                        {
                            environmentPage =
                                EnvironmentPage::Climate;

                            serialPageRefreshRequested =
                                true;
                        }

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
            if (
                isEnvironmentSelected() &&
                (
                    event ==
                        KSJ::InputEvent::RotateLeft ||
                    event ==
                        KSJ::InputEvent::RotateRight
                )
            )
            {
                changeEnvironmentPage(
                    event
                );
                break;
            }

            if (
                event ==
                KSJ::InputEvent::ButtonDown
            )
            {
                currentScreen =
                    AppScreen::Menu;

                serialPageRefreshRequested =
                    false;

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
