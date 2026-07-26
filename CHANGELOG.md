# Changelog

All notable changes to Prototype Box are documented here.

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