# Ramu Prototype Box

Portable embedded development and idea-validation platform.

## Version

Prototype Box v1 — application version `0.1.0`

## Current Capabilities

- SSD1306 OLED display
- Rotary encoder input
- Push-button input
- Simple interactive menu

## Dependencies

- KSJ_Display `v0.1.0`
- KSJ_Input `v0.1.0`

## Wiring

### OLED

| OLED | ESP32 |
|---|---:|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| VCC | 3V3 |
| GND | GND |

### Rotary Encoder

| Encoder | ESP32 |
|---|---:|
| CLK | GPIO 27 |
| DT | GPIO 26 |
| SW | GPIO 25 |
| + | 3V3 |
| GND | GND |

## Behavior

- Rotate left or right to change the selected menu item.
- Press the encoder to select an item.
- Release the encoder to return to the menu.

## Architecture

```text
Physical action
    ↓
KSJ_Input
    ↓
InputEvent
    ↓
Menu logic
    ↓
KSJ_Display
    ↓
OLED

