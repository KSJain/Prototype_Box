# Prototype Box

> **Engineering should feel like magic.**
>
> **Prototype Box is where ideas become reliable.**

Prototype Box is the reference hardware platform for the KSJ ecosystem.

It is the proving ground for RAMU.

Every sensor, interface, controller, and hardware abstraction is first developed, tested, documented, and validated here before becoming part of larger systems such as RAMU Garden and future KSJ projects.

Rather than building one large application, Prototype Box grows by composing small, reusable libraries that each solve one problem well.

Every feature is validated here before it becomes part of a larger system.

---

# Philosophy

> **Build tools that quietly take care of the mundane, so people can spend more time creating, learning, and caring.**

Prototype Box follows the **KSJ Engineering Standard**.

See:

```text
KSJ/
└── ENGINEERING_STANDARD.md
```

---

# Current Software

## Prototype Box Firmware

**Current Release:** **v0.5.1**

## Libraries

| Library         | Version |
| --------------- | ------- |
| KSJ_Display     | v0.1.2  |
| KSJ_Input       | v0.1.0  |
| KSJ_UI          | v0.1.0  |
| KSJ_Environment | v0.2.0  |
| KSJ_Control     | v0.1.0  |
| KSJ_Output      | v0.1.1  |
| KSJ_Storage     | v0.1.0  |

---

# Hardware

## Current Hardware

- ESP32 DOIT DevKit V1
- SSD1306 128×64 OLED Display
- Rotary Encoder
- AHT20 Temperature and Humidity Sensor
- BMP280 Temperature and Pressure Sensor
- Analog LDR
- Micro SD Card Module
- Onboard Status LED

## Planned Hardware

- Four-LED Status Bank
- Relay Module
- MOSFET Driver
- Capacitive Water-Level Sensor
- Peristaltic Pump
- Solenoid Valve
- Capacitive Soil-Moisture Sensor
- CO₂ / VOC Sensor
- Battery and Power Management

---

# Current Features

- Scrollable main menu
- Multi-page Environment interface
- Climate and Light detail pages
- Temperature sensing
- Humidity sensing
- Pressure sensing
- Calibrated ambient-light sensing
- Human-readable light interpretation
- Humidity-control recommendations
- Autonomous onboard LED output
- Context-aware Serial interface
- SD-card persistence
- Persistent boot history
- Local-first operation
- Graceful operation when optional storage is unavailable

---

# Architecture

Prototype Box is composed from reusable libraries.

```text
Observe
    │
    ▼
KSJ_Input
KSJ_Environment

    │
    ▼
Think

KSJ_Control

    │
    ▼
Act

KSJ_Output

    │
    ▼
Present

KSJ_UI
KSJ_Display
Serial Interface

    │
    ▼
Remember

KSJ_Storage
```

Each library has one responsibility.

Products compose libraries.

Libraries never become products.

---

# Environment Pages

The Environment menu contains two live pages.

## Climate

- Temperature
- Humidity
- Pressure
- Humidity-control state

## Light

- Calibrated light percentage
- Raw ADC value
- Human-readable light condition

While Environment is open:

```text
Rotate → switch pages
Press  → return to menu
```

The Serial monitor mirrors the active Environment page every two seconds.

When the user returns to the main menu, live Environment output stops.

---

# Learning Path

The KSJ ecosystem is designed to be learned one brick at a time.

See:

```text
KSJ/
└── LEARNING_PATH.md
```

---

# Build

## Requirements

- PlatformIO
- ESP32 DOIT DevKit V1
- USB Cable

## Build

```bash
git clone https://github.com/KSJain/Prototype_Box.git

cd Prototype_Box

pio run
```

## Upload

```bash
pio run --target upload
```

## Serial Monitor

```bash
pio device monitor
```

---

# Pinout

| Component      | Signal | GPIO |
| -------------- | ------ | ---: |
| OLED           | SDA    |   21 |
| OLED           | SCL    |   22 |
| Rotary Encoder | CLK    |   27 |
| Rotary Encoder | DT     |   26 |
| Rotary Encoder | Switch |   25 |
| Analog LDR     | ADC    |   34 |
| SD Card        | CS     |    5 |
| SD Card        | SCK    |   18 |
| SD Card        | MISO   |   19 |
| SD Card        | MOSI   |   23 |
| Onboard LED    | Output |    2 |

### LDR Voltage Divider
## LDR Voltage Divider

```text
3V3
 │
LDR
 │
├── GPIO34
 │
10 kΩ
 │
GND
```

Calibration depends on the specific LDR, resistor, enclosure, and expected lighting conditions.

---

# Roadmap

## Current

- ✅ Display abstraction
- ✅ Rotary input
- ✅ Menu framework
- ✅ Climate sensing
- ✅ Ambient-light sensing
- ✅ Multi-page Environment interface
- ✅ Context-aware Serial interface
- ✅ Autonomous control decisions
- ✅ Digital output
- ✅ SD-card persistence
- ✅ Persistent boot history

## Next

- ⬜ Four-LED status bank
- ⬜ Relay hardware test
- ⬜ Capacitive water-level sensing
- ⬜ Condensate pump controller
- ⬜ Pump maximum-runtime protection
- ⬜ Pump cooldown protection
- ⬜ 128×32 OLED support
- ⬜ Runtime and maintenance metrics
- ⬜ Event logging

## Future

- ⬜ Digital Gardener
- ⬜ Mushroom Chamber
- ⬜ Basement Water Monitor
- ⬜ KSJ_Motion
- ⬜ ESP-NOW networking
- ⬜ MQTT gateway
- ⬜ OTA updates
- ⬜ Mobile companion app
- ⬜ Web dashboard

---

# Documentation

Additional documentation is available in the **KSJ** folder.

```text
KSJ/
├── ENGINEERING_STANDARD.md
├── LEARNING_PATH.md
├── ROADMAP.md
└── CONTRIBUTING.md
```

---

# FAQ

## Why build libraries instead of one application?

Libraries are easier to understand, test, reuse, diagnose, and improve independently.

Prototype Box demonstrates how they work together as a complete embedded system.

## Why local-first?

Automation should continue operating even when the Internet does not.

Cloud services should extend the system—not become a dependency for basic operation.

## Why provide both OLED and Serial interfaces?

The OLED supports the physical device.

Serial provides diagnostics, accessibility, and a low-cost learning path for someone who owns only an ESP32.

Both surfaces express the same application state.

## Why "Prototype Box"?

Prototype Box is intentionally modular.

It is not the final product.

It is where ideas become reliable.

Every library begins here.

Every interface is validated here.

Every engineering decision is tested here.

Only then does it become part of RAMU.

---

# Vision

Prototype Box is more than an ESP32 development board.

It is the reference implementation of the KSJ embedded ecosystem.

Its purpose is to transform ideas into dependable building blocks that can be reused across products, gardens, workshops, and future systems.

**Build once. Reuse everywhere.**

Engineering should reduce uncertainty.

Engineering should feel like magic.
