# Changelog

All notable changes to Prototype Box are documented here.

## Unreleased — KSJ_Lighting integration

### Added

- Added KSJ_Lighting v0.1.1 as the addressable-light behavior layer
- Added non-blocking StatusService lighting updates
- Added animated ready and humidifying status presentation

### Changed

- StatusService now owns both KSJ_Output::AddressableStrip and KSJ_Lighting::Lighting
- Booting remains solid yellow during setup
- Ready is presented as a slow green breathing pattern
- Humidifying is presented as a blue/cyan Tide pattern
- Fault remains deliberately solid red for unambiguous indication
- Addressable LED data is assigned to GPIO 13

### Preserved

- Existing StatusService public state API
- Humidity-control behavior and thresholds
- OLED UI and navigation
- Environment sensing
- SD storage and append-only logging
- Onboard LED control

### Hardware Validation — 2026-08-09

Verified on the physical Prototype Box:

- KSJ_Lighting v0.1.1 runs continuously through `StatusService::update()`
- Ready state presents a green breathing pattern
- Humidifying state presents the blue/cyan animated state
- Rotary input remains responsive while lighting animates
- Main-menu navigation remains responsive while lighting animates
- Existing humidity/moisture response continues to change status behavior
- Existing ambient-light sensing continues to respond normally
- No direct competing owner of the delegated addressable strip was observed in application code

Still to be explicitly re-validated before promoting this integration to a tagged Prototype Box release:

- Fault-state indication
- SD logging and persistent session/boot count
- Full AHT20/BMP280 regression sweep
- Serial-page regression sweep

## v0.6.0 — 2026-07-27

### Added

- Updated KSJ_Output to v0.3.0
- Added nine-pixel WS2812-compatible status strip
- Added addressable strip initialization during startup
- Added low-brightness status indication
- Added booting, ready, humidifying, and fault states
- Added addressable status state to Serial Climate output

### Status Colors

- Yellow — booting
- Green — ready or humidity satisfied
- Blue — humidification requested
- Red — invalid humidity reading or required-system fault

### Verified

- KSJ_Output v0.3.0 installs successfully
- All nine pixels initialize
- Strip briefly displays yellow during startup
- Strip becomes green after successful startup
- Humidity below 75% changes strip to blue
- Humidity above 82% changes strip to green
- Onboard LED continues following humidity control
- OLED menu and Environment pages remain functional
- Context-aware Serial output remains functional
- SD boot history increments successfully
- LDR remains responsive
- AHT20 and BMP280 remain responsive
- Rotary navigation remains functional

## v0.5.1 — 2026-07-26

### Added

- Updated KSJ_Environment to v0.2.0
- Added calibrated analog LDR sensing
- Added live ambient-light percentage
- Added raw ADC light diagnostics
- Added human-readable light descriptions
- Added two Environment detail pages
- Added Climate and Light page navigation
- Added rotary navigation inside Environment details
- Added explicit OLED page-direction indicators
- Added clear `Btn=Menu` navigation labels

### Changed

- Updated KSJ_Environment to v0.2.1
- Removed accidental raw ADC output from the light-sensor implementation
- Made Serial output context-aware
- Limited live Serial values to the active Environment detail page
- Added a controlled two-second Serial refresh interval
- Added immediate Serial updates when switching Climate and Light pages
- Stopped live Environment output immediately after returning to the menu
- Separated OLED rendering from Serial rendering

### Verified

- Climate page reports temperature, humidity, pressure, and control state
- Light page reports raw ADC, calibrated percentage, condition, and timestamp
- Rotary navigation switches pages on both OLED and Serial
- Button press returns to the menu and stops live output
- Humidity hysteresis continues to control the onboard LED
- SD-card boot history continues to persist

### Preserved

- AHT20 temperature and humidity readings
- BMP280 temperature and pressure readings
- Humidity hysteresis control
- Onboard LED output
- SD-card boot history
- Graceful operation without storage
- Main menu and detail-page navigation