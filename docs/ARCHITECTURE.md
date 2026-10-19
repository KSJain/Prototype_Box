# KSJ Architecture Philosophy

> **Prototype Box discovers bricks.**
> **KSJ Libraries preserve them.**
> **RAMU Nodes put them to work.**

Everything in this ecosystem exists to answer one question:

> **Who should own this responsibility?**

Code moves only after its responsibility becomes clear.

---

# Prototype Box

Prototype Box is the engineering laboratory.

Its purpose is to discover dependable patterns through real hardware.

It is intentionally allowed to experiment.

Not every experiment succeeds.

Not every abstraction survives.

A capability graduates only after repeated validation.

Prototype Box does not optimize for completeness.

It optimizes for learning.

---

# KSJ Libraries

KSJ Libraries preserve proven engineering knowledge.

A library represents one responsibility.

A library should not know which product is using it.

It should work equally well inside:

- Prototype Box
- Water Node
- Garden Node
- Mushroom Node
- Bird Node
- Future devices that do not yet exist

Examples:

- **KSJ_Storage** preserves observations.
- **KSJ_Display** presents information.
- **KSJ_Input** produces user intent.
- **KSJ_Environment** describes the physical world.
- **KSJ_Control** applies policy.
- **KSJ_Output** applies physical output state.
- **KSJ_Lighting** expresses light over time.
- **KSJ_UI** presents application state.

A library earns its place through repetition, not prediction.

---

# RAMU Nodes

RAMU Nodes are complete products.

Each node has a stable identity.

A Water Node remains a Water Node.

A Garden Node remains a Garden Node.

A Mushroom Node remains a Mushroom Node.

Their internal implementation may evolve for years.

Their purpose should not.

Nodes compose proven bricks rather than reimplementing them.

---

# Composition Root

Every application has one place where concrete objects come together.

For Prototype Box, that place is `main.cpp`.

Its responsibility is to assemble the application.

It should answer:

```cpp
setup();

loop();

orchestrate();
```

It should not contain the detailed implementation of every subsystem.

Concrete hardware objects may be created here.

Behavior belongs to the services that own it.

---

# Service Ownership

Each service owns exactly one responsibility.

| Service | Owns |
|---------|------|
| EnvironmentService | Sensor acquisition |
| ControlService | Control policy |
| StatusService | Device status indication |
| StorageService | Persistent memory |
| NavigationService | User location |
| UIService | Presentation |

Services communicate through clearly defined interfaces.

Ownership should be obvious.

---

# Dependency Direction

Dependencies should always point downward.

```text
Prototype Box
        │
        ▼
Application Services
        │
        ▼
KSJ Libraries
        │
        ▼
Hardware
```

Libraries never depend on products.

Products compose libraries.

Hardware never owns application behavior.

---


## Lighting Ownership Example

Prototype Box status lighting follows the same dependency rule:

```text
Prototype Box
      │
      ▼
StatusService
      │
      ▼
KSJ_Lighting
      │
      ▼
KSJ_Output
      │
      ▼
WS2812-compatible hardware
```

`StatusService` decides what device state should look like.

`KSJ_Lighting` decides how that lighting behavior evolves over time.

`KSJ_Output` applies the requested pixel state to hardware.

Once the addressable strip is delegated to `KSJ_Lighting`, no other Prototype Box component writes directly to that strip.

### Hardware validation

On 2026-08-09 this ownership path was exercised on the physical Prototype Box. Animated green/blue status behavior continued while rotary/menu interaction and existing humidity/light responses remained functional. This is evidence that the lighting layer can evolve independently without blocking the application loop.

---

# Graceful Capability Loss

Capabilities may fail independently.

Identity should not.

Missing hardware should reduce capability, not destroy usefulness.

Example:

```text
Storage unavailable.

Application continues.

Memory disabled.
```

The device acknowledges reality and continues providing value.

---

# Extraction Rule

Do not create abstractions because they might be useful.

Extract them only after reality demonstrates repetition.

Observe.

Repeat.

Extract.

Preserve.

---

# Engineering Goal

The goal is not fewer lines of code.

The goal is clearer ownership.

A good `main.cpp` should explain **what** the device does.

Each service should explain **how** one responsibility is fulfilled.

---

# KSJ Engineering Standard

- Safety before features
- Single responsibility
- Non-blocking architecture
- Event-driven design
- SOLID
- DRY
- Hardware abstraction
- State machines where appropriate
- Graceful degradation
- Test before optimize
- Small, consistent progress

---

# Design Principles

> **Prototype Box discovers bricks.
> KSJ Libraries preserve them.
> RAMU Nodes put them to work.**

> **Capabilities may fail independently. Identity should not.**

> **A stable foundation allows skill to accumulate.**

> **Navigation decides where the user is. UI decides how that place appears.**

> **A view may change its clothes without moving the ground beneath the user.**

> **Observe. Repeat. Extract. Preserve.**

> **Every dependable system is built twice: first as an experiment, then as a responsibility.**

> **The best abstractions are discovered—not invented.**

> **A good `main.cpp` should explain what the device does, not how every subsystem works.**

> **Small, consistent progress compounds into dependable systems.**
---

# Semantic Capability Composition

RAMU composition should communicate **meaning**, not remote pin state.

A requester may know that another node advertises:

```text
water.status
water.mist.dispense
```

It should not need to know which GPIO drives the pump relay.

```text
Requester
   |
   | semantic intent
   v
Responsible RAMU node
   |
   | local validation + local safety
   v
Hardware implementation
```

A request is not an action until the responsible node accepts it.

For example:

```text
water.mist.request
zone = 2
duration = 30 s
```

is preferable to:

```text
GPIO17 = HIGH
```

The Water Node remains free to reject the request because of insufficient water,
a local fault, an invalid zone, a bounded-runtime rule, or future independent
safety evidence.

This gives RAMU a stable semantic interface while allowing internal hardware to
evolve.

---

# AI Placement

AI is an optional reasoning/policy layer, not a prerequisite for useful local
hardware behavior.

```text
AI / human / automation
          |
       policy
          |
          v
semantic request
          |
          v
responsible node
          |
 local deterministic authority
```

AI may propose policy, create bounded rules, or interpret observations.

The node that owns the physical consequence retains final execution authority.

> **The physical specialist owns the consequences.**

Removing the AI, server, LAN, or Internet should not remove the node's useful
local responsibility.

# Prototype Box Node Laboratory

Prototype Box retains its original product identity while also serving as the
RAMU Node Laboratory.

> **Prototype Box should gain laboratory powers without losing its original invitation to explore.**

The boot sequence, status-light behavior, four-item root menu, and rotary-driven
interaction are part of the laboratory's stable human interface. Experimental
features must be good house guests: they may extend the laboratory but must not
displace the behavior that already teaches a finder how to use it.

Current root navigation remains:

```text
Dashboard
Environment
Settings
About
```

Node-development tools live beneath `Settings -> Node Lab`.

## Laboratory roles

The Node Lab separates three useful responsibilities.

### Inspector

Inspector addresses a real peer and exercises what that peer claims it can do.
It should distinguish discovery, reachability, authorization, availability,
request acceptance, and execution result rather than collapsing them into a
single "connected" state.

A future peer view may therefore show an out-of-range node as known but
unavailable rather than deleting it from the operator's mental model.

### Stub

A Stub represents a node or capability that is not physically present yet.
Stubs are legitimate test instruments, not fake physical evidence.

A stub must always identify its origin explicitly:

```text
transport actor: prototype-box
logical subject: garden-stub-01
implementation:  stub
simulated:       true
```

Physical observations use the complementary form:

```text
implementation: physical
simulated:      false
```

This provenance is part of the data contract and should survive into SD logs,
network messages, analysis tools, and future RAMU-HOME history. The goal is to
make the "DNA ancestry" of data obvious enough that future versions never need
to infer whether an observation was born from hardware, a stub, replay, or
another source.

### Scenario

Scenario is a future role built above Stubs. A scenario is a named preset or a
bounded sequence of changes that can reproduce useful operating conditions.
Examples include a dry greenhouse afternoon, falling reservoir level, or a
network peer moving in and out of range.

Scenarios should manipulate semantic values and events rather than raw GPIO.

## Capability contract

A capability being supported does not mean it is currently available, and
availability does not imply that every caller is authorized to request it.

```text
supported
    ↓
available now
    ↓
caller authorized
    ↓
request valid
    ↓
responsible specialist decides
    ↓
execution / explicit denial
```

Decision provenance should bubble upstream without being rewritten into a
vague generic failure:

```json
{
  "op":"water.mist.request",
  "ok":false,
  "decision_by":"water-01",
  "reason":"safety.low_water"
}
```

The node that owns the physical consequence owns the decision. Upstream nodes
may explain or display that decision, but should preserve who made it and why.

A safety stop is not necessarily a permanent failure. For example, low water
may stop the current mist cycle; a later request may succeed after the local
water-availability condition recovers.

## Garden and Soil responsibility boundary

Garden Node is planned as an environmental observer and local garden
orchestrator when RAMU-HOME is absent. Its core observations include local
climate and light, plus local UI/history as those capabilities mature.

Soil sensing is intentionally separated into a future `Soil_Node`, potentially
solar powered. Soil_Node owns soil observations; Garden Node may consume them.

```text
Soil_Node ───────────────┐
                        │ semantic observation
Garden Sense ───────────┤
                        ▼
                   Garden Node
                observe + local policy
                        │
                        ▼
                  specialist requests
```

RAMU-HOME may later assign or revise orchestration roles, and local UI or FOB
may provide human overrides, but each specialist retains authority over the
physical responsibility it owns.

## Laboratory promotion rule

Prototype Box may discover a useful communication or test pattern, but that
pattern does not become a shared KSJ/RAMU abstraction simply because it worked
once.

```text
Observe.
Repeat.
Exercise with more than one node type.
Extract.
Preserve.
```

Today's peer may be Water Node. Tomorrow it may be a light, feeder, soil,
climate, or entirely new specialist. The laboratory should remain useful across
all of them.
