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