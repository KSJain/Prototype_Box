#include "UIService.h"

UIService::UIService(
    KSJ::SSD1306Display& display,
    PrototypeBox::Menu& menu,
    NavigationService& navigation,
    EnvironmentService& environment,
    ControlService& control,
    StatusService& status,
    CommsLabService& comms,
    PeerBindingHostService& peers
)
    : _display(display),
      _menu(menu),
      _navigation(navigation),
      _environment(environment),
      _control(control),
      _status(status),
      _comms(comms),
      _peers(peers)
{
}

void UIService::renderMenu()
{
    _menu.draw(
        _display
    );
}

void UIService::renderDetail()
{
    if (_navigation.isEnvironmentSelected())
    {
        renderEnvironment();
        return;
    }

    if (_navigation.isSettingsSelected())
    {
        renderSettings();
        return;
    }

    renderGenericDetail();
}

void UIService::renderEnvironment()
{
    switch (_navigation.environmentPage())
    {
        case NavigationPage::Climate:
            renderClimate();
            break;

        case NavigationPage::Light:
            renderLight();
            break;
    }
}

void UIService::printCurrentEnvironmentPage()
{
    if (
        !_navigation.isDetailScreen() ||
        !_navigation.isEnvironmentSelected()
    )
    {
        return;
    }

    switch (_navigation.environmentPage())
    {
        case NavigationPage::Climate:
            printClimateToSerial();
            break;

        case NavigationPage::Light:
            printLightToSerial();
            break;
    }
}

const char* UIService::lightConditionText(
    const KSJ::LightReading& reading
) const
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

void UIService::renderClimate()
{
    const KSJ::EnvironmentReading& climate =
        _environment.climate();

    const KSJ::EnvironmentReading& pressure =
        _environment.pressure();

    _display.clear();
    _display.setTextSize(1);

    _display.print(
        0,
        0,
        "Environment 1/2 >"
    );

    _display.drawLine(
        0,
        10,
        127,
        10
    );

    if (climate.temperatureValid)
    {
        const String value =
            String(
                climate.temperatureC,
                1
            );

        _display.print(
            0,
            15,
            "Temp"
        );

        _display.print(
            42,
            15,
            value.c_str()
        );

        _display.print(
            78,
            15,
            "C"
        );
    }
    else
    {
        _display.print(
            0,
            15,
            "Temp      --"
        );
    }

    if (climate.humidityValid)
    {
        const String value =
            String(
                climate.humidityPercent,
                1
            );

        _display.print(
            0,
            27,
            "Humidity"
        );

        _display.print(
            54,
            27,
            value.c_str()
        );

        _display.print(
            90,
            27,
            "%"
        );
    }
    else
    {
        _display.print(
            0,
            27,
            "Humidity  --"
        );
    }

    if (pressure.pressureValid)
    {
        const String value =
            String(
                pressure.pressureHpa,
                0
            );

        _display.print(
            0,
            39,
            "Pressure"
        );

        _display.print(
            54,
            39,
            value.c_str()
        );

        _display.print(
            84,
            39,
            "hPa"
        );
    }
    else
    {
        _display.print(
            0,
            39,
            "Pressure  --"
        );
    }

    _display.print(
        0,
        51,
        "Control"
    );

    _display.print(
        48,
        51,
        _control.statusText()
    );

    _display.print(
        78,
        51,
        "Btn=Menu"
    );

    _display.update();
}

void UIService::renderLight()
{
    const KSJ::LightReading& reading =
        _environment.light();

    _display.clear();
    _display.setTextSize(1);

    _display.print(
        0,
        0,
        "< Environment 2/2"
    );

    _display.drawLine(
        0,
        10,
        127,
        10
    );

    _display.print(
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

        _display.print(
            42,
            16,
            value.c_str()
        );

        _display.print(
            78,
            16,
            "%"
        );
    }
    else
    {
        _display.print(
            42,
            16,
            "--"
        );
    }

    _display.print(
        0,
        30,
        "Mood:"
    );

    _display.print(
        36,
        30,
        lightConditionText(
            reading
        )
    );

    _display.print(
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

        _display.print(
            30,
            44,
            value.c_str()
        );
    }
    else
    {
        _display.print(
            30,
            44,
            "--"
        );
    }

    _display.print(
        0,
        56,
        "Rotate | Btn=Menu"
    );

    _display.update();
}

void UIService::renderSettings()
{
    _display.clear();
    _display.setTextSize(1);

    _display.print(0, 0, "Settings");
    _display.drawLine(0, 10, 127, 10);

    const SettingsAction action =
        _navigation.settingsAction();

    const char* name = "?";

    switch (action)
    {
        case SettingsAction::NodeLab:
            name = "Node Lab";
            break;
        case SettingsAction::Peers:
            name = "Peers";
            break;
        case SettingsAction::PairPeer:
            name = "Pair Peer";
            break;
        case SettingsAction::Back:
            name = "Back";
            break;
        case SettingsAction::Count:
        default:
            break;
    }

    _display.print(0, 20, ">");
    _display.print(12, 20, name);

    if (action == SettingsAction::Peers)
    {
        _display.print(
            12,
            34,
            _peers.hasBinding()
                ? _peers.peerId().c_str()
                : "No bound peers"
        );
    }
    else if (action == SettingsAction::PairPeer)
    {
        _display.print(12, 34, "Intentional BLE window");
    }
    else if (action == SettingsAction::NodeLab)
    {
        _display.print(12, 34, "ESP-NOW experiment");
    }

    _display.print(0, 57, "Rotate | Btn=Select");
    _display.update();
}

void UIService::renderPeer()
{
    _display.clear();
    _display.setTextSize(1);

    _display.print(0, 0, "Remembered Peer");
    _display.drawLine(0, 10, 127, 10);

    if (!_peers.hasBinding())
    {
        _display.print(0, 18, "No active binding");
        _display.print(0, 34, "Pair Peer in Settings");
        _display.print(0, 57, "Rotate | Back");
        _display.update();
        return;
    }

    _display.print(
        0,
        14,
        _peers.peerId().c_str()
    );

    const PeerAction action =
        _navigation.peerAction();

    const char* actionName = "?";

    switch (action)
    {
        case PeerAction::Overview:
            actionName = "Overview";
            break;
        case PeerAction::Capabilities:
            actionName = "Capabilities";
            break;
        case PeerAction::Unbind:
            actionName = "Unbind";
            break;
        case PeerAction::Back:
            actionName = "Back";
            break;
        case PeerAction::Count:
        default:
            break;
    }

    _display.print(0, 27, ">");
    _display.print(12, 27, actionName);

    if (action == PeerAction::Overview)
    {
        _display.print(0, 40, "Binding");
        _display.print(
            42,
            40,
            _peers.bindingId().c_str()
        );
    }
    else if (action == PeerAction::Capabilities)
    {
        int count = 0;
        const String& caps =
            _peers.capabilities();

        if (caps.length() > 0)
        {
            count = 1;
            for (size_t i = 0; i < caps.length(); ++i)
            {
                if (caps[i] == ',')
                {
                    ++count;
                }
            }
        }

        String summary =
            String(count) +
            " advertised";

        _display.print(
            0,
            40,
            summary.c_str()
        );
    }
    else if (action == PeerAction::Unbind)
    {
        _display.print(0, 40, "Revoke local authority");
    }
    else
    {
        _display.print(0, 40, "Return to Settings");
    }

    _display.print(
        0,
        57,
        action == PeerAction::Back
            ? "Btn=Back"
            : "Rotate | Btn=Select"
    );

    _display.update();
}

void UIService::renderNodeLab()
{
    _display.clear();
    _display.setTextSize(1);

    _display.print(0, 0, "RAMU Node Lab C");
    _display.print(90, 0, String(_comms.channel()).c_str());
    _display.drawLine(0, 10, 127, 10);

    _display.print(0, 13, nodeLabRoleName(_navigation.nodeLabRole()));
    _display.print(92, 13, _comms.peerSeen() ? "P+" : "P-");

    _display.print(0, 24, ">");
    _display.print(10, 24, commsLabActionName(_navigation.commsAction()));

    _display.print(0, 35, "State");
    _display.print(36, 35, _comms.lastState());

    _display.print(0, 46, "Why");
    _display.print(30, 46, _comms.lastReason());

    if (_navigation.commsAction() == CommsLabAction::Back)
    {
        _display.print(0, 57, "Btn=Settings");
    }
    else if (_comms.awaitingResponse())
    {
        _display.print(0, 57, "Waiting for peer");
    }
    else
    {
        _display.print(0, 57, "Rotate | Btn=Send");
    }

    _display.update();
}

void UIService::renderGenericDetail()
{
    _display.clear();
    _display.setTextSize(1);

    _display.print(
        0,
        0,
        "Selected:"
    );

    _display.drawLine(
        0,
        10,
        127,
        10
    );

    _display.setTextSize(2);

    _display.print(
        0,
        20,
        _navigation.selectedItem()
    );

    _display.setTextSize(1);

    _display.print(
        0,
        52,
        "Press to return"
    );

    _display.update();
}

void UIService::printClimateToSerial()
{
    const KSJ::EnvironmentReading& climate =
        _environment.climate();

    const KSJ::EnvironmentReading& pressure =
        _environment.pressure();

    Serial.println();

    Serial.println(
        "Environment > Climate"
    );

    Serial.println(
        "------------------------"
    );

    Serial.print("Temperature: ");

    if (climate.temperatureValid)
    {
        Serial.print(
            climate.temperatureC,
            1
        );

        Serial.println(" C");
    }
    else
    {
        Serial.println("INVALID");
    }

    Serial.print("Humidity:    ");

    if (climate.humidityValid)
    {
        Serial.print(
            climate.humidityPercent,
            1
        );

        Serial.println(" %");
    }
    else
    {
        Serial.println("INVALID");
    }

    Serial.print("Pressure:    ");

    if (pressure.pressureValid)
    {
        Serial.print(
            pressure.pressureHpa,
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
        _control.statusText()
    );

    Serial.print("Status LED:  ");

    Serial.println(
        _status.statusText()
    );

    Serial.println(
        "Rotate = Page | Press = Menu"
    );

    Serial.println(
        "------------------------"
    );
}

void UIService::printLightToSerial()
{
    const KSJ::LightReading& reading =
        _environment.light();

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
        lightConditionText(
            reading
        )
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