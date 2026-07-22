#include "Menu.h"

namespace PrototypeBox
{

const char* const Menu::ITEMS[Menu::ITEM_COUNT] =
{
    "Dashboard",
    "Environment",
    "Settings",
    "About"
};

Menu::Menu()
    : _selectedIndex(0),
      _selectionPending(false)
{
}

void Menu::update(KSJ::InputEvent event)
{
    switch (event)
    {
        case KSJ::InputEvent::RotateLeft:
        {
            if (_selectedIndex == 0)
            {
                _selectedIndex = ITEM_COUNT - 1;
            }
            else
            {
                --_selectedIndex;
            }

            break;
        }

        case KSJ::InputEvent::RotateRight:
        {
            _selectedIndex =
                (_selectedIndex + 1) % ITEM_COUNT;

            break;
        }

        case KSJ::InputEvent::ButtonDown:
        {
            _selectionPending = true;
            break;
        }

        case KSJ::InputEvent::ButtonUp:
        case KSJ::InputEvent::None:
        default:
        {
            break;
        }
    }
}

void Menu::draw(KSJ::IGraphicsDevice& display) const
{
    display.clear();

    display.setTextSize(1);
    display.print(0, 0, "Prototype Box v1");

    display.drawLine(
        0,
        10,
        127,
        10
    );

    for (uint8_t index = 0; index < ITEM_COUNT; ++index)
    {
        const int16_t y =
            18 + static_cast<int16_t>(index * 14);

        if (index == _selectedIndex)
        {
            display.print(0, y, ">");
        }
        else
        {
            display.print(0, y, " ");
        }

        display.print(
            12,
            y,
            ITEMS[index]
        );
    }

    display.update();
}

uint8_t Menu::selectedIndex() const
{
    return _selectedIndex;
}

const char* Menu::selectedItem() const
{
    return ITEMS[_selectedIndex];
}

bool Menu::consumeSelection()
{
    if (!_selectionPending)
    {
        return false;
    }

    _selectionPending = false;
    return true;
}

}