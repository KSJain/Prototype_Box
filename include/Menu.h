#pragma once

#include <Arduino.h>

#include <IGraphicsDevice.h>
#include <InputEvent.h>

namespace PrototypeBox
{

class Menu
{
public:
    static constexpr uint8_t ITEM_COUNT = 4;

    Menu();

    void update(KSJ::InputEvent event);

    void draw(KSJ::IGraphicsDevice& display) const;

    uint8_t selectedIndex() const;

    const char* selectedItem() const;

    bool consumeSelection();

private:
    static const char* const ITEMS[ITEM_COUNT];

    uint8_t _selectedIndex;
    bool _selectionPending;
};

}