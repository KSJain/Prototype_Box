#pragma once

#include <Arduino.h>

#include <KSJ_SSD1306Display.h>
#include <LightReading.h>

#include "ControlService.h"
#include "EnvironmentService.h"
#include "Menu.h"
#include "NavigationService.h"
#include "StatusService.h"

class UIService
{
public:
    UIService(
        KSJ::SSD1306Display& display,
        PrototypeBox::Menu& menu,
        NavigationService& navigation,
        EnvironmentService& environment,
        ControlService& control,
        StatusService& status
    );

    void renderMenu();

    void renderDetail();

    void renderEnvironment();

    void printCurrentEnvironmentPage();

private:
    const char* lightConditionText(
        const KSJ::LightReading& reading
    ) const;

    void renderClimate();

    void renderLight();

    void renderGenericDetail();

    void printClimateToSerial();

    void printLightToSerial();

    KSJ::SSD1306Display& _display;
    PrototypeBox::Menu& _menu;
    NavigationService& _navigation;
    EnvironmentService& _environment;
    ControlService& _control;
    StatusService& _status;
};