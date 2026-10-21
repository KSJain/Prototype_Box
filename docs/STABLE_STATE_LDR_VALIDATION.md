# KSJ_StableState — Prototype Box LDR Validation

## Purpose

Validate the beta `KSJ_StableState` threshold tracker inside a real ESP32 composition using a noisy physical sensor.

This experiment intentionally tests only the generic stable-state mechanism:

```text
real scalar
    ↓
ThresholdStateTracker
    ↓
Unknown / Below / Above
```

No actuator behavior and no domain-specific mapping are involved.

## Composition

The Prototype Box LDR is sampled through the existing `KSJ_Environment` path:

```text
LDR
    ↓
KSJ::AnalogLightSensor
    ↓
KSJ::LightReading
    ↓
rawValue + rawValid
    ↓
KSJ::StableState::ThresholdStateTracker
```

The tracker remains outside `EnvironmentService` and is composed at the application level.

## Test Configuration

Existing Prototype Box LDR calibration:

```text
LDR_DARK_RAW              = 650
LDR_BRIGHT_RAW            = 3650
LDR_HIGHER_RAW_IS_BRIGHTER = true
LDR_INTERVAL_MS           = 500 ms
```

Stable-state experiment configuration:

```text
Below threshold = 1400
Above threshold = 2900
Persistence     = 1000 ms
```

These thresholds are laboratory values chosen to make physical transitions easy to reproduce. They are not intended to define universal meanings such as `Dark` or `Bright`.

## Observed Behavior

### ABOVE → BELOW

With the tracker already established as `ABOVE`, the LDR value was reduced.

Observed sequence:

```text
raw ≈ 2335
state=ABOVE
candidate=ABOVE

raw ≈ 1735
state=ABOVE
candidate=ABOVE

raw ≈ 1175
state=ABOVE
candidate=BELOW

raw ≈ 1103
state=ABOVE
candidate=BELOW

raw ≈ 1095
state=BELOW
candidate=BELOW
changed=true
```

The stable state did not change immediately when the lower threshold was crossed.

`BELOW` first became the candidate state and was committed only after the persistence requirement was satisfied.

### BELOW → ABOVE

With the tracker established as `BELOW`, the LDR was exposed to stronger light.

Observed sequence:

```text
raw ≈ 3007
state=BELOW
candidate=ABOVE

raw ≈ 2993
state=BELOW
candidate=ABOVE

raw ≈ 3002
state=ABOVE
candidate=ABOVE
changed=true
```

The reverse transition also respected persistence.

## Hysteresis Observation

Values between the configured thresholds:

```text
1400 < raw < 2900
```

did not manufacture a new state.

When `ABOVE` had already been established, readings such as approximately:

```text
2335
2085
1735
1873
1802
2448
2180
```

preserved the existing `ABOVE` state.

This matches the intended rule:

> Hysteresis may preserve established knowledge; it must not manufacture knowledge that was never established.

## Noise Behavior

The real LDR produced normal analog variation around otherwise stable lighting conditions.

Examples included small fluctuations around approximately:

```text
2990–3015
1000–1100
3900–4050
```

These fluctuations did not cause state chatter.

The tracker remained stable unless the signal crossed the opposite threshold and remained there long enough to satisfy persistence.

## Result

The beta `KSJ_StableState` tracker successfully demonstrated:

- initial generic `Unknown / Below / Above` state behavior
- lower-threshold transition persistence
- upper-threshold transition persistence
- hysteresis preserving established state
- resistance to normal analog sensor noise
- clean composition with an existing ESP32 sensor service
- no requirement for domain-specific meaning inside the brick

## Evidence Chain

```text
Water v1 field-matured behavior
        ↓
KSJ_StableState extraction
        ↓
13 / 13 native automated tests
        ↓
Prototype Box ESP32 integration
        ↓
real LDR physical validation
```

## Current Conclusion

The experiment supports keeping the current beta abstraction:

```text
scalar + validity + monotonic time
        ↓
thresholds + hysteresis + persistence
        ↓
Unknown / Below / Above
```

Specialist meaning should remain outside the brick.

Examples:

```text
Water:
    Below → Dry
    Above → Wet

Battery:
    Below → Low
    Above → Healthy
```

No API change was required as a result of this first hardware composition test.

## Status

**Validation passed.**

The brick remains beta and should continue to be challenged by additional compositions and field replay before its API is considered stable.