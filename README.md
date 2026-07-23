# Prototype Box

> **Engineering should feel like magic.**
>
> **Prototype Box is where ideas become reliable.**

Prototype Box is the reference hardware platform for the KSJ ecosystem.

It is the proving ground for RAMU.

Every sensor, interface, controller, and hardware abstraction is first developed, tested, documented, and validated here before becoming part of larger systems such as RAMU Garden and future KSJ projects.

Rather than building one large application, Prototype Box grows by composing small, reusable libraries that each solve one problem well.

Every feature is tested here first.

---

# Philosophy

> **Build tools that quietly take care of the mundane, so people can spend more time creating, learning, and caring.**

Prototype Box follows the **KSJ Engineering Standard**.

See:

```
KSJ/
└── ENGINEERING_STANDARD.md
```

---

# Current Software

### Prototype Box Firmware

**Current Release:** **v0.3.0**

## Libraries

| Library         | Version |
| --------------- | ------- |
| KSJ_Display     | v0.1.2  |
| KSJ_Input       | v0.1.0  |
| KSJ_UI          | v0.1.0  |
| KSJ_Environment | v0.1.0  |
| KSJ_Control     | v0.1.0  |
| KSJ_Output      | v0.1.1  |

---

# Hardware

## Current Hardware

* ESP32 DOIT DevKit V1
* SSD1306 OLED Display
* Rotary Encoder
* AHT20 Temperature & Humidity Sensor
* BMP280 Pressure Sensor
* SD Card Module
* Onboard Status LED

## Planned Hardware

* WS2812 Status Ring
* Relay Module
* MOSFET Driver
* Peristaltic Pump
* Solenoid Valve
* Water Level Sensor
* Capacitive Soil Moisture Sensors
* Ambient Light Sensor
* CO₂ / VOC Sensor

---

# Architecture

Prototype Box is composed from independent libraries.

```
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
```

Each library has one responsibility.

Products compose libraries.

Libraries never become products.

---

# Learning Path

The KSJ ecosystem is designed to be learned one brick at a time.

```
KSJ_Display

↓

KSJ_Input

↓

KSJ_UI

↓

KSJ_Environment

↓

KSJ_Control

↓

KSJ_Output

↓

Prototype Box

↓

RAMU Garden
```

Each library introduces one major concept while remaining useful on its own.

---

# Build

## Requirements

* PlatformIO
* ESP32 DOIT DevKit V1
* USB Cable

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

*Coming soon.*

Prototype Box maintains a standardized hardware mapping so examples remain consistent across every KSJ library.

---

# Roadmap

## Current

* ✅ Display abstraction
* ✅ Rotary input
* ✅ Menu framework
* ✅ Environment sensing
* ✅ Autonomous control decisions
* ✅ Physical digital outputs
* ✅ Modular architecture

## Next

* ⬜ KSJ_Storage
* ⬜ KSJ_Time
* ⬜ KSJ_Comms
* ⬜ Metrics & Statistics
* ⬜ Event Logger
* ⬜ ESP-NOW Networking

## Future

* ⬜ Digital Gardener
* ⬜ Mushroom Chamber
* ⬜ Basement Water Monitor
* ⬜ OTA Updates
* ⬜ Mobile Companion App
* ⬜ Web Dashboard

---

# Documentation

Additional documentation is available under the **KSJ** folder.

```
KSJ/

ENGINEERING_STANDARD.md

LEARNING_PATH.md

ROADMAP.md

CONTRIBUTING.md
```

---

# Gallery

Coming soon.

Future revisions will include photographs, wiring diagrams, hardware revisions, and demonstrations.

---

# FAQ

### Why build libraries instead of one application?

Libraries are easier to understand, test, reuse, and improve independently.

Prototype Box demonstrates how they work together as a complete system.

---

### Why local-first?

Automation should continue operating even when the Internet does not.

Cloud services should extend the system—not become a dependency for its basic operation.

---

### Why "Prototype Box"?

Prototype Box is intentionally modular.

It is not the final product.

It is the place where ideas become reliable.

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
