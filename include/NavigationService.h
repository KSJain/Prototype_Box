#pragma once

#include <Arduino.h>
#include <InputEvent.h>
#include "CommsLabTypes.h"
#include "Menu.h"

enum class NavigationScreen : uint8_t
{
    Menu,
    Detail,
    NodeLab,
    Peer
};

enum class NavigationPage : uint8_t
{
    Climate,
    Light
};

enum class SettingsAction : uint8_t
{
    NodeLab = 0,
    Peers,
    PairPeer,
    Back,
    Count
};

enum class PeerAction : uint8_t
{
    Overview = 0,
    Capabilities,
    Unbind,
    Back,
    Count
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
    bool isNodeLabScreen() const;
    bool isPeerScreen() const;
    bool isEnvironmentSelected() const;
    bool isSettingsSelected() const;

    NavigationPage environmentPage() const;
    SettingsAction settingsAction() const;
    PeerAction peerAction() const;
    CommsLabAction commsAction() const;
    NodeLabRole nodeLabRole() const;

    const char* selectedItem() const;

    bool consumeMenuChanged();
    bool consumeDetailOpened();
    bool consumePageChanged();
    bool consumeReturnedToMenu();
    bool consumeNodeLabOpened();
    bool consumePeerOpened();
    bool consumePeerActionRequested();
    bool consumeCommsActionRequested();
    bool consumePairingRequested();

private:
    void updateMenu(
        KSJ::InputEvent event
    );

    void updateDetail(
        KSJ::InputEvent event
    );

    void updateNodeLab(
        KSJ::InputEvent event
    );

    void updatePeer(
        KSJ::InputEvent event
    );

    void rotateSettingsAction(
        bool right
    );

    void rotatePeerAction(
        bool right
    );

    void rotateCommsAction(
        bool right
    );

    PrototypeBox::Menu& _menu;

    NavigationScreen _screen =
        NavigationScreen::Menu;

    NavigationPage _environmentPage =
        NavigationPage::Climate;

    SettingsAction _settingsAction =
        SettingsAction::NodeLab;

    PeerAction _peerAction =
        PeerAction::Overview;

    CommsLabAction _commsAction =
        CommsLabAction::Status;

    bool _menuChanged = false;
    bool _detailOpened = false;
    bool _pageChanged = false;
    bool _returnedToMenu = false;
    bool _nodeLabOpened = false;
    bool _peerOpened = false;
    bool _peerActionRequested = false;
    bool _commsActionRequested = false;
    bool _pairingRequested = false;
    bool _settingsButtonDown = false;
};
