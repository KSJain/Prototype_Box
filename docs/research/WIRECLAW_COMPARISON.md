# Research Note — RAMU and WireClaw

Date reviewed: 2026-10-14

This note records an architectural comparison between RAMU and WireClaw after
reviewing the WireClaw project overview and official documentation.

This is not a performance benchmark and is not intended to rank one project
above the other. The useful result is identifying where an independent project
validates architectural instincts already emerging in RAMU, and where the two
systems deliberately choose different centers of gravity.

## Sources reviewed

- Video overview supplied during RAMU research:
  https://www.youtube.com/watch?v=EFd0kDA0Oww
- WireClaw project site:
  https://wireclaw.io/
- WireClaw architecture:
  https://wireclaw.io/architecture
- WireClaw under-the-hood notes:
  https://wireclaw.io/learn-more
- WireClaw source repository:
  https://github.com/M64GitHub/WireClaw

## High-level comparison

| | RAMU | WireClaw |
|---|---|---|
| Fundamental unit | Domain specialist | Generic AI-capable ESP device |
| Local autonomy | Core architectural requirement | Persistent local rule engine |
| AI location | May live above/across nodes; not required by every node | Each WireClaw contains an LLM client; cloud or local LLM endpoint |
| Runtime without AI | Nodes continue their local responsibility | Created rules continue locally without LLM calls |
| Hardware ownership | Node owns meaning, hardware translation, and local safety | Device registry/rules expose generic sensors and actuators; project also exposes direct GPIO/HAL tools |
| Inter-node language | Semantic capabilities and intents | NATS values, request/reply, tools, remote chat |
| Near-field transport | ESP-NOW is being validated for local composition | NATS over IP/Wi-Fi is the documented multi-device path |
| Failure philosophy | Dependencies may disappear without taking useful hardware with them | Rule loop continues when the AI is not involved; network-dependent features still depend on their transport |
| Safety-critical actuation | Receiving specialist makes the final local decision | Published model is more generic and includes direct actuator/GPIO operations |
| Persistent automation | Planned as policy/rule capability above domain-owned execution | Implemented local rule engine and non-blocking rule chains |
| Human language | Intended higher-level interface, optional to node operation | First-class interaction surface via Telegram/Serial/NATS |
| Device naming | Capability-oriented node identity is emerging | Persistent named device registry already implemented |

## Strong architectural overlap

WireClaw independently validates several directions already present in RAMU.

### 1. AI should not remain in the runtime control loop

WireClaw describes its design as two cooperating loops: an on-demand AI loop
creates/configures rules, while a local rule loop evaluates those rules without
requiring continued LLM calls.

That maps naturally onto the RAMU direction:

```text
AI / human intent
      |
      v
policy or bounded semantic request
      |
      v
domain specialist
      |
      v
local deterministic execution
```

A useful RAMU phrasing is:

> AI may author or suggest policy. The responsible node owns execution.

### 2. Persistent local automation matters

WireClaw rules survive reboots and execute locally. RAMU has independently been
moving toward the same property through local node autonomy, SD evidence,
persistent configuration, and bounded state machines.

### 3. Named devices are better than pin-number thinking

WireClaw's device registry gives sensors and actuators names. RAMU is heading
further toward semantic capabilities:

```text
water-01
  capabilities:
    water.status
    water.mist.dispense

garden-01
  capabilities:
    environment.observe
    light.observe
    garden.policy

lamp-04
  capabilities:
    light.output
```

The shared lesson is that higher layers should reason about meaning rather than
physical pin assignments.

### 4. Non-blocking action chains are valuable

WireClaw supports multi-step rules with non-blocking delays. RAMU water behavior
needs the same execution property:

```text
OPEN V2
   |
   | settle
   v
PUMP ON
   |
   | bounded duration
   v
PUMP OFF
   |
   | depressurize
   v
CLOSE V2
```

The sequence is useful. Ownership is the important difference: RAMU intends the
Water Node to own this sequence rather than exposing it as a generic chain of
raw GPIO writes.

## RAMU's different center of gravity

RAMU is developing around **distributed physical specialists with local
responsibility and semantic composition**.

A higher layer should not need to know that a pump relay happens to be attached
to GPIO17.

Bad RAMU interface:

```text
GPIO17 = HIGH
```

Preferred RAMU interface:

```text
water.mist.request
zone = 2
duration = 30 s
```

The Water Node then decides whether the request is valid, whether water is
available, which valve/pump sequence is required, and which local safety limits
must remain authoritative.

This distinction becomes more important as consequences increase.

A generic LED can tolerate a generic `gpio_write()` mental model.

A water specialist may need to consider:

- reservoir availability
- valve state
- pump state
- maximum runtime
- local faults
- flow evidence
- operator stop
- future independent safety witnesses

The requester should not be required to reconstruct those responsibilities.

## Capability, container membership, and temporary role

RAMU should continue keeping these separate:

```text
CAPABILITY
what a node can do

MEMBERSHIP
which local garden/room/system currently includes it

ROLE
what responsibility it has been assigned for this situation
```

A node may advertise:

```text
water.mist.dispense
```

without advertising private internal implementation or granting direct actuator
access.

## AI placement principle

WireClaw demonstrates that an LLM can be brought extremely close to hardware.
RAMU should retain the option without making it mandatory.

```text
             optional AI
                 |
       policy / interpretation
                 |
                 v
Garden Brain / RAMU-HOME / local agent
                 |
          semantic intent
                 |
        +--------+--------+
        |                 |
        v                 v
   Water Node         Light Node
 local authority     local authority
```

The useful property is not where the AI physically runs.

The useful property is:

> Removing the AI must not remove the node's useful local responsibility.

## Research conclusion

WireClaw does not make RAMU redundant.

It provides useful independent evidence that several RAMU directions are sound:

- persistent local execution
- named devices rather than pin-first thinking
- non-blocking action chains
- optional AI-created automation
- multi-device composition

RAMU should borrow patterns where they improve the system while preserving its
own strongest constraint:

> **The physical specialist owns the consequences.**

And the broader RAMU principle remains:

> **A dependency should be allowed to die without taking the useful hardware
> with it.**
