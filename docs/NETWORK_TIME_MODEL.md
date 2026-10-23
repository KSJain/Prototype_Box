# Prototype Box Network + Time Model

## Principle

Missing Internet must never mean missing observation.

Operational timing remains monotonic (`millis()`). Wi-Fi and UTC are opportunistic services.

## Clock model

Each SD event/telemetry record already carries `uptime_ms` through `KSJ_Storage`.
When SNTP first establishes a trustworthy wall clock, Prototype Box records:

```json
{
  "event": "TIME_SYNC",
  "uptime_ms": 43821,
  "data": {
    "utc_epoch": 1792695572,
    "sync_uptime_ms": 43821,
    "source": "ntp",
    "valid": true
  }
}
```

RAMU-HOME can reconstruct wall time for records in the session using that anchor:

```text
record_utc ~= sync_utc + (record_uptime_ms - sync_uptime_ms)
```

Do not use reconstructed UTC for real-time safety deadlines. Runtime deadlines continue to use monotonic time.

## Wi-Fi credentials

Tracked:

```text
include/config/NetworkConfig.h
include/config/NetworkConfig.example.h
```

Ignored:

```text
include/config/NetworkConfig.local.h
```

Create the local file by copying the example and filling in SSID/password.

## ESP-NOW coexistence

The old lab forced ESP-NOW channel 1. This conflicted with infrastructure Wi-Fi because an associated ESP32 must share the AP's radio channel.

The new behavior is:

- offline: park radio on configured fallback channel (default 1)
- associated to Wi-Fi: radio follows AP channel
- ESP-NOW broadcast peer uses channel `0`, meaning current radio channel

This preserves the previous offline lab while allowing Wi-Fi coexistence.

A remote ESP-NOW peer still has to be on the same current channel. Future binding metadata can retain/reconcile transport-channel knowledge.
