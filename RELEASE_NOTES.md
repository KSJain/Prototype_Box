# Prototype Box v0.5.1

Prototype Box now treats Serial as a deliberate second presentation surface rather than a debug stream.

## Highlights

- Context-aware Serial Environment pages
- Two-second live refresh while Environment is open
- Immediate output when changing Climate and Light pages
- Live output stops when returning to the main menu
- Removed continuous raw LDR debug values
- Updated KSJ_Environment dependency to v0.2.1

## Verified Behavior

```text
Main Menu
→ navigation events only

Environment > Climate
→ temperature, humidity, pressure, and control state

Environment > Light
→ raw ADC, calibrated light percentage, condition, and timestamp

Press = Menu
→ live Environment output stops
```

This patch preserves the existing OLED interface, humidity control, onboard LED behavior, and persistent SD-card boot history.
