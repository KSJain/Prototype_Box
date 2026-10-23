# RAMU Intentional Peer Binding — First Physical Milestone

Date: 2026-10-23

## Proven physical relationship

Prototype Box stable identity:

`copper-willow-wren`

Lab Node stable identity:

`indigo-window-moth`

First successful binding identity:

`calm-stone-fox`

Observed capability manifest:

- `system.identity.read`
- `system.capabilities.read`
- `node.status`
- `node.identify`
- `lab.led.pattern`
- `lab.stub.observation`

## Ceremony proven on hardware

```text
physical long press
↓
pairing.requested
↓
temporary BLE visibility
↓
public human verification code
↓
Prototype Box discovers armed peer
↓
human confirms
↓
identity exchange
↓
capability exchange
↓
same binding identity persisted on both ends
↓
pairing window closes
↓
Lab Node returns quiet
```

This validates the intended dark-forest behavior: BLE discovery is not a normal
operating beacon. Visibility exists only during explicit pairing intent.

## Identity rule

Human labels, deployments, cartridges and transports may change. Stable machine
identity does not.

For Prototype Box, `copper-willow-wren` is permanent machine identity and must
survive firmware updates, deployment changes and future cartridge experiments.

## Relationship model

Identity, deployment, capability, authorization and transport are separate.

The current six-digit pairing code is public human-verification material. It is
not a cryptographic password and the current binding must not be described as
cryptographically authenticated.

## Remembered-peer persistence

The active relationship remains in NVS for boot-time availability.

Prototype Box also mirrors relationship state to SD:

- `/RAMU_PEER_ACTIVE.json` — current active relationship snapshot
- `/RAMU_PEER_EVENTS.ndjson` — append-only relationship history

An unbind removes local authorization and the active snapshot but appends a
tombstone event. History is deliberately preserved.

```text
Disconnect != Unbind != Erase history
```

## Prototype Box UI direction

```text
Settings
├── Node Lab
├── Peers
│   └── indigo-window-moth
│       ├── Overview
│       ├── Capabilities
│       ├── Unbind
│       └── Back
├── Pair Peer
└── Back
```

The first remembered-peer UI is inspection-first. It does not claim a capability
is invokable until a normal-operation transport exists for that relationship.

## Next capability-driven UI layer

Once normal-operation routing exists, the Prototype Box should derive available
screens/actions from advertised capability vocabulary rather than hard-code node
types.

Example:

```text
system.identity.read        -> Identity / Info
system.capabilities.read    -> Capabilities
node.status                 -> Status screen
node.identify               -> Identify control
lab.led.pattern             -> LED Pattern controls
lab.stub.observation        -> Observation / Info
```

Transport remains an implementation detail. The UI asks for capability intent;
the specialist node decides how that intent maps to local behavior.

## Technical debt intentionally deferred

Classic `ESP32 BLE Arduino` is flash-heavy and emits non-fatal remote descriptor
warnings during the successful bind. Do not optimize this before the relationship
model is stable. NimBLE is a future candidate.

The current Prototype Box binding store supports one active peer. The UI/storage
shape should evolve to a multi-peer registry after the single-peer lifecycle is
fully proven.
