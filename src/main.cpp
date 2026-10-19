#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <cstring>

#include <KSJ_SSD1306Display.h>

#include <InputEvent.h>
#include <KSJ_RotaryEncoder.h>

#include "EnvironmentService.h"
#include "CommsLabService.h"

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
#include "UIService.h"
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

namespace UIRefreshConfig
{
    constexpr uint32_t SERIAL_INTERVAL_MS =
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

CommsLabService commsService;

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

UIService uiService(
    display,
    menu,
    navigationService,
    environmentService,
    controlService,
    statusService,
    commsService
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

    const bool commsReady =
        commsService.begin();

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

    uiService.renderMenu();

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

    Serial.print("Comms lab: ");
    Serial.println(
        commsReady
            ? "ready"
            : "unavailable"
    );

    if (commsReady)
    {
        Serial.print("Comms MAC: ");
        Serial.println(commsService.localMac());

        Serial.print("Comms channel: ");
        Serial.println(commsService.channel());
    }

    Serial.print("Persistent boot count: ");
    Serial.println(bootCount);

    Serial.println("Prototype Box: ready");
}

void loop() {

    const uint32_t nowMs = millis();

    statusService.update();

    commsService.update(nowMs);

    if (commsService.consumeChanged())
    {
        const String payload =
            commsService.stateJson();

        storageService.logEvent(
            nowMs,
            "COMMS_STATE",
            payload.c_str()
        );

        if (navigationService.isNodeLabScreen())
        {
            uiService.renderNodeLab();
        }
    }

    updateEnvironmentSensors(nowMs);

    storageService.update(
        nowMs,
        environmentService.light()
    );

    storageReady =
        storageService.isReady();

    if (
        navigationService.isDetailScreen() &&
        navigationService.isEnvironmentSelected()
    )
    {
        const bool serialIntervalElapsed =
            nowMs -
                previousSerialPageMs >=
            UIRefreshConfig::SERIAL_INTERVAL_MS;

        if (
            serialPageRefreshRequested ||
            serialIntervalElapsed
        )
        {
            previousSerialPageMs =
                nowMs;

            serialPageRefreshRequested =
                false;

            uiService.printCurrentEnvironmentPage();
        }
    }
    else
    {
        serialPageRefreshRequested =
            false;
    }

    if (navigationService.isDetailScreen() && navigationService.isEnvironmentSelected() && environmentChanged) {
        uiService.renderEnvironment();
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

        uiService.renderMenu();
    }

    if (
        navigationService.consumeDetailOpened()
    )
    {
        Serial.print("Opened: ");
        Serial.println(
            navigationService.selectedItem()
        );

        uiService.renderDetail();

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
        if (navigationService.isNodeLabScreen())
        {
            uiService.renderNodeLab();
            serialPageRefreshRequested = false;
        }
        else
        {
            uiService.renderDetail();
            serialPageRefreshRequested =
                navigationService
                    .isEnvironmentSelected();
        }
    }

    if (
        navigationService.consumeNodeLabOpened()
    )
    {
        Serial.println("Opened: RAMU Node Lab");
        uiService.renderNodeLab();
        serialPageRefreshRequested = false;
    }

    if (
        navigationService
            .consumeCommsActionRequested()
    )
    {
        const CommsLabAction action =
            navigationService.commsAction();

        const bool sent =
            commsService.request(
                action,
                nowMs
            );

        String payload = "{\"action\":\"";
        payload += commsLabActionName(action);
        payload += "\",\"role\":\"";
        payload += nodeLabRoleName(commsLabActionRole(action));
        payload += "\",\"sent\":";
        payload += sent ? "true" : "false";
        payload += "}";

        storageService.logEvent(
            nowMs,
            "COMMS_REQUEST",
            payload.c_str()
        );

        uiService.renderNodeLab();
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

        uiService.renderMenu();

        environmentChanged = false;
    }

}