# Prototype Box v0.6.0

Prototype Box now includes a nine-pixel addressable status strip powered by KSJ_Output v0.3.0.

The strip provides immediate physical feedback without requiring the OLED or Serial monitor.

## Status Language

```text
Yellow → Booting
Green  → Ready
Blue   → Humidification requested
Red    → Fault
```

## Verified Integration

* Nine-pixel WS2812-compatible strip
* Addressable strip on GPIO12
* AHT20 temperature and humidity sensing
* BMP280 temperature and pressure sensing
* LDR ambient-light sensing
* Humidity hysteresis control
* Onboard LED output
* OLED navigation
* Context-aware Serial interface
* SD-card boot persistence

The boot log reached firmware `0.6.0` while preserving all existing system behavior.

This release adds a new presentation surface without changing the sensor, control, storage, or menu architecture.
