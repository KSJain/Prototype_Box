#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <cstring>

#include <KSJ_SSD1306Display.h>

#include <InputEvent.h>
#include <KSJ_RotaryEncoder.h>

#include <LightReading.h>
#include "EnvironmentService.h"

#include <ControlAction.h>
#include "ControlService.h"

#include "StatusService.h"

#include <KSJ_SDStorage.h>
#include <StorageInfo.h>
#include <StorageResult.h>
#include <StorageStatus.h>
#include "StorageService.h"

#include "Menu.h"
#include "NavigationService.h"
#include "PrototypeBoxConfig.h"


namespace OutputPins
{
    constexpr uint8_t ONBOARD_LED =
        2;
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
}

namespace AddressableConfig
{
    constexpr uint16_t PIXEL_COUNT =
        8;

    constexpr uint8_t BRIGHTNESS =
        24;
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

EnvironmentService environmentService(
    Wire
);

ControlService controlService(
    OutputPins::ONBOARD_LED,
    true
);

StatusService statusService(
    AddressableConfig::PIXEL_COUNT,
    Pins::ADDRESSABLE_LED_DATA,
    AddressableConfig::BRIGHTNESS
);

SPIClass sdSpi(VSPI);

KSJ::SDStorage storage(
    sdSpi,
    StoragePins::CS,
    StorageConfig::SPI_FREQUENCY_HZ
);

PrototypeBox::Menu menu;

NavigationService navigationService(
    menu
);

bool environmentChanged = false;

bool storageReady = false;

uint32_t bootCount = 0;

uint32_t previousSerialPageMs = 0;

bool serialPageRefreshRequested = false;

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

StorageService storageService(
    sdSpi,
    storage
);

const char* humidityControlText()
{
    return controlService.statusText();
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

void updateHumidityControl()
{
    const KSJ::ControlAction action =
        controlService.update(
            environmentService.climate()
        );

    if (!controlService.isValid())
    {
        statusService.setFault();

        return;
    }

    switch (action)
    {
        case KSJ::ControlAction::TurnOn:
            statusService.setHumidifying();
            break;

        case KSJ::ControlAction::TurnOff:
            statusService.setReady();
            break;

        case KSJ::ControlAction::Hold:
        default:
            break;
    }
}

void printClimatePageToSerial()
{
    const KSJ::EnvironmentReading& ahtReading =
        environmentService.climate();

    const KSJ::EnvironmentReading& bmpReading =
        environmentService.pressure();

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

    Serial.print("Status LED:  ");

    Serial.println(
        statusService.statusText()
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
        environmentService.light();

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
        !navigationService.isDetailScreen() ||
        !navigationService.isEnvironmentSelected()
    )
    {
        return;
    }

    switch (navigationService.environmentPage())
    {
        case NavigationPage::Climate:
            printClimatePageToSerial();
            break;

        case NavigationPage::Light:
            printLightPageToSerial();
            break;
    }
}

void updateSerialInterface(
    uint32_t nowMs
)
{
    if (
        !navigationService.isDetailScreen() ||
        !navigationService.isEnvironmentSelected()
    )
    {
        serialPageRefreshRequested =
            false;

        return;
    }

    const bool intervalElapsed =
        nowMs -
            previousSerialPageMs >=
        SerialUI::LIVE_REFRESH_INTERVAL_MS;

    if (
        !serialPageRefreshRequested &&
        !intervalElapsed
    )
    {
        return;
    }

    previousSerialPageMs =
        nowMs;

    serialPageRefreshRequested =
        false;

    printActiveEnvironmentPageToSerial();
}

void drawClimatePage()
{
    const KSJ::EnvironmentReading& ahtReading =
        environmentService.climate();

    const KSJ::EnvironmentReading& bmpReading =
        environmentService.pressure();

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

        display.print(
            0,
            15,
            "Temp"
        );

        display.print(
            42,
            15,
            value.c_str()
        );

        display.print(
            78,
            15,
            "C"
        );
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

        display.print(
            0,
            27,
            "Humidity"
        );

        display.print(
            54,
            27,
            value.c_str()
        );

        display.print(
            90,
            27,
            "%"
        );
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

        display.print(
            0,
            39,
            "Pressure"
        );

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

    display.print(
        0,
        51,
        "Control"
    );

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
        environmentService.light();

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

    display.print(
        0,
        16,
        "Light:"
    );

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

        display.print(
            78,
            16,
            "%"
        );
    }
    else
    {
        display.print(
            42,
            16,
            "--"
        );
    }

    display.print(
        0,
        30,
        "Mood:"
    );

    display.print(
        36,
        30,
        lightConditionText(reading)
    );

    display.print(
        0,
        44,
        "Raw:"
    );

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
        display.print(
            30,
            44,
            "--"
        );
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
    switch (navigationService.environmentPage())
    {
        case NavigationPage::Climate:
            drawClimatePage();
            break;

        case NavigationPage::Light:
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
        navigationService.selectedItem()
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
    if (navigationService.isEnvironmentSelected())
    {
        drawEnvironmentScreen();
        return;
    }

    drawGenericDetailScreen();
}

void updateEnvironmentSensors(
    uint32_t nowMs
)
{
    environmentService.update(
        nowMs
    );

    if (
        environmentService.consumeClimateChanged()
    )
    {
        updateHumidityControl();
        environmentChanged = true;
    }

    if (
        environmentService.consumePressureChanged()
    )
    {
        environmentChanged = true;
    }

    if (
        environmentService.consumeLightChanged()
    )
    {
        environmentChanged = true;
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println(Board::NAME);

    Serial.print("Version: ");
    Serial.println(Board::VERSION);

    Serial.print("MCU: ");
    Serial.println(Board::MCU);

    const bool statusReady =
        statusService.begin();

    Wire.begin(Pins::SDA, Pins::SCL);
    Wire.setClock(100000);

    if (!display.begin()) {
        Serial.println("Display initialization failed.");
        statusService.setFault();
        while (true) { delay(1000); }
    }

    encoder.begin();

    const bool controlReady =
        controlService.begin();

    const bool environmentReady =
        environmentService.begin();

    storageReady =
    storageService.begin(
        Board::VERSION,
        Board::MCU
    );

    bootCount =
    storageService.sessionNumber();

    Serial.println("Humidity control:");

    Serial.print("  ON below ");
    Serial.print(
        SensorConfig::HUMIDITY_ON_BELOW_PERCENT,
        1
    );
    Serial.println("%");

    Serial.print("  OFF above ");
    Serial.print(
        SensorConfig::HUMIDITY_OFF_ABOVE_PERCENT,
        1
    );
    Serial.println("%");

    const bool requiredSystemsReady = environmentReady;

    if (!requiredSystemsReady) {
        statusService.setFault();
    } else {
        statusService.setReady();
    }

    menu.draw(display);

    Serial.println("Display: ready");

    Serial.println("Input: ready");

    Serial.println("Environment: ready");

    Serial.println("Light: ready");

    Serial.print("Control: ");
    Serial.println(
        controlReady
            ? "ready"
            : "unavailable"
    );

    Serial.print("Output: ");
    Serial.println(
        controlReady
            ? "ready"
            : "unavailable"
    );

    Serial.print("Addressable strip: ");
    Serial.println(
        statusReady
            ? "ready"
            : "unavailable"
    );

    Serial.print("Storage: ");
    Serial.println(storageReady ? "ready" : "unavailable");

    Serial.print("Persistent boot count: ");
    Serial.println(bootCount);

    Serial.println("Prototype Box: ready");
}

void loop() {

    const uint32_t nowMs = millis();

    updateEnvironmentSensors(nowMs);

    storageService.update(
        nowMs,
        environmentService.light()
    );

    storageReady =
        storageService.isReady();

    updateSerialInterface(nowMs);

    if (navigationService.isDetailScreen() && navigationService.isEnvironmentSelected() && environmentChanged) {
        drawEnvironmentScreen();
        environmentChanged = false;
    }

    const KSJ::InputEvent event =
        encoder.update();

    if (event == KSJ::InputEvent::None)
    {
        return;
    }

    navigationService.update(
        event
    );

    if (
        navigationService.consumeMenuChanged()
    )
    {
        Serial.print("Menu: ");
        Serial.println(
            navigationService.selectedItem()
        );

        menu.draw(display);
    }

    if (
        navigationService.consumeDetailOpened()
    )
    {
        Serial.print("Opened: ");
        Serial.println(
            navigationService.selectedItem()
        );

        drawDetailScreen();

        environmentChanged = false;

        if (
            navigationService
                .isEnvironmentSelected()
        )
        {
            serialPageRefreshRequested =
                true;
        }
    }

    if (
        navigationService.consumePageChanged()
    )
    {
        drawEnvironmentScreen();

        serialPageRefreshRequested =
            true;
    }

    if (
        navigationService.consumeReturnedToMenu()
    )
    {
        serialPageRefreshRequested =
            false;

        Serial.println(
            "Returned to menu"
        );

        menu.draw(display);

        environmentChanged = false;
    }

}