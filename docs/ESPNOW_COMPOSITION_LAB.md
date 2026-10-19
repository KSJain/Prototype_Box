# Prototype Box Node Lab ↔ Garden Water ESP-NOW Composition Lab

## Goal

Use Prototype Box as a persistent RAMU laboratory rather than temporarily
turning it into a Garden or Water product.

The experiment validates two laboratory roles against the Garden Water bring-up
endpoint:

```text
INSPECTOR
    request status
    request onboard LED patterns
    observe explicit acceptance / denial
    later: request bounded relay clicks with 12 V disconnected

STUB
    explicitly represent garden-stub-01
    send simulated environmental observations
    preserve stub provenance end-to-end
```

The experiment intentionally earns actuator authority in stages:

```text
status request
    ↓
onboard LED patterns
    ↓
explicit stub observations
    ↓
locked relay denial
    ↓
relay clicks with 12 V actuator supply disconnected
    ↓
later: bounded powered water actions
```

The current branch does **not** grant powered water authority.

## Preserve Prototype Box identity

This experiment must not replace the original Prototype Box interaction model.

> **Prototype Box should gain laboratory powers without losing its original invitation to explore.**

The root menu remains:

```text
Dashboard
Environment
Settings
About
```

Node Lab lives under:

```text
Settings
    ↓
Node Lab
```

The current Node Lab action carousel is deliberately small. A future UI may
expand into `Peers`, `Capabilities`, `Requests`, `Stubs`, `Events`, and `Back`
after the underlying behavior earns that structure.

## Radio setup

Both lab firmwares use ESP-NOW channel `1` and operate in Wi-Fi station mode
without joining an access point.

Broadcast is a bring-up mechanism, not the permanent RAMU trust model. After
basic transport is proven, explicit peer identity/pairing is the next intended
communications increment.

## Inspector actions

Current Inspector actions:

```text
STATUS
LED OFF
LED SLOW
LED FAST
LED TRIPLE
RELAY 1
RELAY 2
RELAY 3
RELAY 4
```

The OLED reports peer visibility, current laboratory role/action, and the most
recent remote state.

## Stub actions and provenance

Current Stub presets provide small garden-like observations without pretending
they came from real sensors:

```text
GDN DRY
GDN HUMID
GDN DARK
```

Each request explicitly identifies its ancestry. Example:

```json
{"v":1,"kind":"request","from":"prototype-box","to":"lab-node-01","op":"lab.stub.observation","as":"garden-stub-01","impl":"stub","sim":true,"cap":"environment.observe","rh":37.0,"temp_c":25.2,"light":420}
```

There is intentionally no soil field in the Garden stub. Soil observations are
planned to belong to a separate future Soil_Node, potentially solar powered.

A stub may be useful enough to drive real decision code, but its data must never
silently masquerade as physical evidence.

## Scenario role

Scenario is defined but not yet implemented as a sequencer. It is intended to
be a named preset or time-varying test above Stub values, for example:

```text
DRY AFTERNOON
RH 60 → 52 → 43 → 36
```

or:

```text
R20 available
    ↓
request accepted
    ↓
R20 becomes unavailable
    ↓
current mist cycle safety-stops
    ↓
later recovery can allow a new request
```

A safety stop is preferred terminology here over "terminate" because the
condition may be recoverable.

## Endpoint safety gate

The Garden Water endpoint is compiled with:

```cpp
constexpr bool ALLOW_RELAY_CLICKS = false;
```

Leave this `false` for the first status/LED/stub pass.

After those pass:

1. disconnect the external 12 V actuator supply
2. verify the relay board alone is powered from its intended logic supply
3. set `ALLOW_RELAY_CLICKS = true`
4. rebuild and flash the endpoint
5. request relay pulses one channel at a time

The endpoint bounds each requested relay pulse to a maximum of one second.
Prototype Box currently requests 250 ms pulses.

## Current actuator mapping

```text
IN1 / GPIO13 -> Valve 1
IN2 / GPIO4  -> Valve 2
IN3 / GPIO16 -> Valve 3
IN4 / GPIO17 -> Pump
```

No semantic request exposes those GPIO numbers to Prototype Box.

## Evidence and decision provenance

Prototype Box records communication-state events through its existing
`KSJ_Storage` JSONL session logger.

The endpoint creates its own append-only JSONL session under:

```text
/EXP0002/
```

Physical endpoint records identify themselves as:

```json
{"implementation":"physical","simulated":false}
```

Stub-origin records preserve the request source and simulated subject.

A responsible denial should preserve who decided and why. The generic shape we
are developing toward is:

```json
{
  "op":"water.mist.request",
  "ok":false,
  "decision_by":"water-01",
  "reason":"safety.low_water"
}
```

Upstream software may present a friendlier message, but should not erase that
provenance.

## Promotion rule

Do not extract `KSJ_Comms` merely because this experiment works once.

First validate the same semantic request/response and provenance pattern across
at least two real node relationships.

> Observe. Repeat. Extract. Preserve.
