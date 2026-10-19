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

        case NavigationScreen::NodeLab:
            updateNodeLab(event);
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

bool NavigationService::isNodeLabScreen() const
{
    return _screen == NavigationScreen::NodeLab;
}

bool NavigationService::isEnvironmentSelected() const
{
    return std::strcmp(
        _menu.selectedItem(),
        "Environment"
    ) == 0;
}

bool NavigationService::isSettingsSelected() const
{
    return std::strcmp(
        _menu.selectedItem(),
        "Settings"
    ) == 0;
}

NavigationPage NavigationService::environmentPage() const
{
    return _environmentPage;
}

SettingsAction NavigationService::settingsAction() const
{
    return _settingsAction;
}

CommsLabAction NavigationService::commsAction() const
{
    return _commsAction;
}

NodeLabRole NavigationService::nodeLabRole() const
{
    return commsLabActionRole(_commsAction);
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

bool NavigationService::consumeNodeLabOpened()
{
    const bool value = _nodeLabOpened;
    _nodeLabOpened = false;
    return value;
}

bool NavigationService::consumeCommsActionRequested()
{
    const bool value = _commsActionRequested;
    _commsActionRequested = false;
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

                if (isSettingsSelected())
                {
                    _settingsAction =
                        SettingsAction::NodeLab;
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


void NavigationService::rotateSettingsAction()
{
    _settingsAction =
        _settingsAction == SettingsAction::NodeLab
        ? SettingsAction::Back
        : SettingsAction::NodeLab;
}

void NavigationService::rotateCommsAction(
    bool right
)
{
    const uint8_t count =
        static_cast<uint8_t>(
            CommsLabAction::Count
        );

    uint8_t index =
        static_cast<uint8_t>(
            _commsAction
        );

    if (right)
    {
        index =
            static_cast<uint8_t>(
                (index + 1U) % count
            );
    }
    else
    {
        index =
            index == 0
                ? count - 1U
                : index - 1U;
    }

    _commsAction =
        static_cast<CommsLabAction>(
            index
        );
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

    if (isSettingsSelected())
    {
        if (
            event == KSJ::InputEvent::RotateLeft ||
            event == KSJ::InputEvent::RotateRight
        )
        {
            rotateSettingsAction();
            _pageChanged = true;
            return;
        }

        if (event == KSJ::InputEvent::ButtonDown)
        {
            if (_settingsAction == SettingsAction::NodeLab)
            {
                _screen = NavigationScreen::NodeLab;
                _commsAction = CommsLabAction::Status;
                _nodeLabOpened = true;
            }
            else
            {
                _screen = NavigationScreen::Menu;
                _returnedToMenu = true;
            }

            return;
        }

        return;
    }

    if (event == KSJ::InputEvent::ButtonDown)
    {
        _screen = NavigationScreen::Menu;
        _returnedToMenu = true;
    }
}

void NavigationService::updateNodeLab(
    KSJ::InputEvent event
)
{
    if (event == KSJ::InputEvent::RotateLeft)
    {
        rotateCommsAction(false);
        _pageChanged = true;
        return;
    }

    if (event == KSJ::InputEvent::RotateRight)
    {
        rotateCommsAction(true);
        _pageChanged = true;
        return;
    }

    if (event == KSJ::InputEvent::ButtonDown)
    {
        if (_commsAction == CommsLabAction::Back)
        {
            _screen = NavigationScreen::Detail;
            _pageChanged = true;
        }
        else
        {
            _commsActionRequested = true;
        }
    }
}
