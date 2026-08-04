#include "NavigationService.h"

#include <cstring>

NavigationService::NavigationService(
    PrototypeBox::Menu& menu
)
    : _menu(menu)
{
}

void NavigationService::update(
    KSJ::InputEvent event
)
{
    switch (_screen)
    {
        case NavigationScreen::Menu:
            updateMenu(event);
            break;

        case NavigationScreen::Detail:
            updateDetail(event);
            break;
    }
}

bool NavigationService::isMenuScreen() const
{
    return _screen == NavigationScreen::Menu;
}

bool NavigationService::isDetailScreen() const
{
    return _screen == NavigationScreen::Detail;
}

bool NavigationService::isEnvironmentSelected() const
{
    return std::strcmp(
        _menu.selectedItem(),
        "Environment"
    ) == 0;
}

NavigationPage NavigationService::environmentPage() const
{
    return _environmentPage;
}

const char* NavigationService::selectedItem() const
{
    return _menu.selectedItem();
}

bool NavigationService::consumeMenuChanged()
{
    const bool value = _menuChanged;
    _menuChanged = false;
    return value;
}

bool NavigationService::consumeDetailOpened()
{
    const bool value = _detailOpened;
    _detailOpened = false;
    return value;
}

bool NavigationService::consumePageChanged()
{
    const bool value = _pageChanged;
    _pageChanged = false;
    return value;
}

bool NavigationService::consumeReturnedToMenu()
{
    const bool value = _returnedToMenu;
    _returnedToMenu = false;
    return value;
}

void NavigationService::updateMenu(
    KSJ::InputEvent event
)
{
    _menu.update(event);

    switch (event)
    {
        case KSJ::InputEvent::RotateLeft:
        case KSJ::InputEvent::RotateRight:
            _menuChanged = true;
            break;

        case KSJ::InputEvent::ButtonDown:
            if (_menu.consumeSelection())
            {
                _screen = NavigationScreen::Detail;

                if (isEnvironmentSelected())
                {
                    _environmentPage =
                        NavigationPage::Climate;
                }

                _detailOpened = true;
            }

            break;

        case KSJ::InputEvent::ButtonUp:
        case KSJ::InputEvent::None:
        default:
            break;
    }
}

void NavigationService::updateDetail(
    KSJ::InputEvent event
)
{
    if (
        isEnvironmentSelected() &&
        (
            event == KSJ::InputEvent::RotateLeft ||
            event == KSJ::InputEvent::RotateRight
        )
    )
    {
        _environmentPage =
            _environmentPage == NavigationPage::Climate
            ? NavigationPage::Light
            : NavigationPage::Climate;

        _pageChanged = true;
        return;
    }

    if (event == KSJ::InputEvent::ButtonDown)
    {
        _screen = NavigationScreen::Menu;
        _returnedToMenu = true;
    }
}