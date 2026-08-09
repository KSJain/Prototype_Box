# Prototype Box — KSJ_Lighting Integration Validation

**Integration:** KSJ_Lighting v0.1.1  
**Hardware:** Prototype Box v1 / ESP32 DOIT DevKit V1  
**Date:** 2026-08-09  
**Status:** Integration hardware pass successful; remaining regression checks tracked below

> **Objective**
>
> Confirm that KSJ_Lighting v0.1.1 can own the Prototype Box addressable status strip without disturbing sensing, control, input, UI, or storage.

---

## Integration Boundary

```text
Control / startup state
        │
        ▼
StatusService
        │
        ▼
KSJ_Lighting
        │
        ▼
KSJ_Output::AddressableStrip
        │
        ▼
8-pixel WS2812-compatible strip
GPIO 13
```

`StatusService` remains the application-facing owner of status indication.

`KSJ_Lighting` owns runtime manipulation of the delegated addressable strip.

No other Prototype Box component should write directly to that strip.

---

## Status Language

| System state | Lighting behavior | Purpose |
|---|---|---|
| Booting | Solid yellow | Dependable startup indication |
| Ready | Green Breathe | Quiet heartbeat and continuous tick validation |
| Humidifying | Blue/cyan Tide | Visible active-control state |
| Fault | Solid red | Unambiguous fault indication |

Fault intentionally remains static so fault visibility does not depend on animation timing.

---

## Hardware Evidence — 2026-08-09

The following behavior was directly observed on the physical Prototype Box after integrating `KSJ_Lighting v0.1.1`:

- [x] Addressable lighting runs continuously through the normal application loop
- [x] Ready state visibly breathes green
- [x] Blue/cyan active-control lighting is visible when the control state changes
- [x] Lighting returns between blue and green state presentation as conditions change
- [x] Rotary input remains responsive while lighting animates
- [x] Main-menu navigation remains responsive while lighting animates
- [x] Existing humidity/moisture response remains functional
- [x] Existing ambient-light response remains functional
- [x] No obvious blocking behavior is introduced by the lighting animation

This test is the first Prototype Box confirmation that the lighting brick can animate while existing interactive and environmental behavior continues to operate.

---

## Build / Dependency Record

The integration is configured for:

- PlatformIO
- ESP32 DOIT DevKit V1
- `KSJ_Output v0.3.0`
- `KSJ_Lighting v0.1.1`
- 8-pixel WS2812-compatible strip
- GPIO 13 addressable-data output

Repository configuration should continue to pin the two output layers explicitly in `platformio.ini`.

---

## Ownership Validation

The intended runtime path is:

```text
StatusService → KSJ_Lighting → KSJ_Output → hardware
```

`StatusService` contains the delegated `KSJ::AddressableStrip` and `KSJ::Lighting` instances. Application code changes status through `StatusService`; it does not independently draw pixels.

A useful repository check is:

```bash
grep -R "\.fill\|\.setPixel\|\.show" src include \
  --exclude=StatusService.cpp
```

There should be no Prototype Box application path directly manipulating the delegated status strip.

---

## Remaining Explicit Regression Checks

These were not explicitly exercised in the reported 2026-08-09 lighting integration pass and should remain open rather than being inferred as passed:

- [ ] Boot begins with solid yellow
- [ ] Fault state becomes solid red
- [ ] AHT20 readings are explicitly re-checked
- [ ] BMP280 readings are explicitly re-checked
- [ ] SD logging is explicitly re-checked during active animation
- [ ] Persistent boot/session count is explicitly re-checked
- [ ] Serial Environment-page refresh is explicitly re-checked
- [ ] Power-loss/storage behavior remains unchanged

---

## Release Gate

The integration is safe to commit as a known working development milestone based on the observed interactive hardware test.

Before promoting it to a tagged Prototype Box release, complete the remaining explicit regression checks above and record their results rather than assuming them.

---

## Engineering Result

The test validates the architectural boundary:

```text
Product state decides why.
StatusService decides what.
KSJ_Lighting evolves light over time.
KSJ_Output applies physical state.
```

The addressable strip can now express device state continuously without taking over the Prototype Box event loop.
