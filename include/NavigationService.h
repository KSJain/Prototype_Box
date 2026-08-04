#pragma once

#include <Arduino.h>
#include <InputEvent.h>

#include "Menu.h"

enum class NavigationScreen : uint8_t
{
    Menu,
    Detail
};

enum class NavigationPage : uint8_t
{
    Climate,
    Light
};

class NavigationService
{
public:
    explicit NavigationService(
        PrototypeBox::Menu& menu
    );

    void update(
        KSJ::InputEvent event
    );

    bool isMenuScreen() const;
    bool isDetailScreen() const;
    bool isEnvironmentSelected() const;

    NavigationPage environmentPage() const;

    const char* selectedItem() const;

    bool consumeMenuChanged();
    bool consumeDetailOpened();
    bool consumePageChanged();
    bool consumeReturnedToMenu();

private:
    void updateMenu(
        KSJ::InputEvent event
    );

    void updateDetail(
        KSJ::InputEvent event
    );

    PrototypeBox::Menu& _menu;

    NavigationScreen _screen =
        NavigationScreen::Menu;

    NavigationPage _environmentPage =
        NavigationPage::Climate;

    bool _menuChanged = false;
    bool _detailOpened = false;
    bool _pageChanged = false;
    bool _returnedToMenu = false;
};