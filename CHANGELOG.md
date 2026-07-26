# Changelog

All notable changes to Prototype Box are documented here.

## v0.5.0 — 2026-07-26

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

- Updated Prototype Box rotary encoder pins
- Improved Environment screen spacing and readability
- Simplified humidity-control display to `ON` and `OFF`
- Reduced pressure precision on OLED to conserve display space

### Preserved

- AHT20 temperature and humidity readings
- BMP280 temperature and pressure readings
- Humidity hysteresis control
- Onboard LED output
- SD-card boot history
- Graceful operation without storage
- Main menu and detail-page navigation