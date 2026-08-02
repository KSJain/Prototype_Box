# Prototype Box v0.7.0 — Storage Validation Plan

> **Objective**
>
> Can I lose power at any moment without losing meaningful observations?

Prototype Box v0.7.0 validates dependable, append-only observation logging while preserving every existing Prototype Box capability.

The storage layer is an observer. It must never control, delay, or disable sensing, display, input, or automation.

---

## Scope

### Preserve Existing Behavior

- [ ] Existing menus remain unchanged
- [ ] Existing rotary input remains unchanged
- [ ] Existing OLED behavior remains unchanged
- [ ] Existing LEDs remain unchanged
- [ ] Existing sensors remain unchanged
- [ ] Existing humidity-control logic remains unchanged
- [ ] Existing boot-count persistence remains unchanged

### Add Session Logging

- [ ] SD card mounts successfully
- [ ] A new session starts automatically after each boot
- [ ] Every session receives a unique session identifier
- [ ] Session records begin at zero-based uptime
- [ ] A `SESSION_START` record is appended
- [ ] LDR telemetry is written every 5 seconds
- [ ] Light-zone changes are written immediately as events
- [ ] Signal stability changes are written immediately as events
- [ ] Records use append-only JSON Lines
- [ ] Sequence numbers increase monotonically within a session
- [ ] Logging failures are reported without stopping the application

---

## Power-Loss Validation

1. Start Prototype Box.
2. Confirm the SD card is ready.
3. Confirm a new session file is created.
4. Allow several telemetry records to be written.
5. Change room lighting several times.
6. Confirm light and stability events are recorded.
7. Pull USB power while logging is active.
8. Reconnect power.
9. Confirm the boot count increments.
10. Confirm a new session file is created.
11. Confirm the previous session file remains readable.
12. Confirm every complete JSON line remains parseable.
13. Ignore or safely detect any final partial line.
14. Confirm all menus, sensors, controls, LEDs, and display functions still operate.

---

## Pass Criteria

Prototype Box v0.7.0 passes when:

- [ ] Power can be removed during active logging without corrupting earlier complete records
- [ ] The previous session remains readable after reboot
- [ ] A new session begins automatically after reboot
- [ ] Boot count remains persistent
- [ ] Telemetry resumes without manual intervention
- [ ] Storage failure does not interrupt sensing, UI, input, or control
- [ ] Existing Prototype Box behavior remains unchanged

---

## Failure Policy

If storage is unavailable or a write fails:

```text
Storage unavailable
        ↓
Continue sensing
Continue UI
Continue input
Continue control
        ↓
Report LOGGING DEGRADED
```

Logging records safety.

Logging is not safety.

---

## Deferred Work

The following are intentionally outside v0.7.0:

- Wi-Fi
- NTP time synchronization
- RTC support
- Remote file browsing
- Serial service console
- Graphs and dashboards
- Condensate-controller integration
- Metrics interpretation
- Automatic calibration

These features may use the same storage API later without changing the append-only session model.

---

## Engineering Intent

> Every observation should be useful more than once.

Prototype Box v0.7.0 turns temporary sensor readings into durable engineering evidence for debugging, calibration, trend analysis, future graphs, and later RAMU products.
