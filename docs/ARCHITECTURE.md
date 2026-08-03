# KSJ Architecture Philosophy

> Prototype Box discovers bricks.  
> KSJ Libraries preserve them.  
> RAMU Nodes put them to work.

## Prototype Box

Prototype Box is the engineering laboratory.

It explores sensors, services, interfaces, failure modes, and reusable patterns.

Its purpose is not to contain every future capability. Its purpose is to prove the smallest dependable version of a capability before that knowledge is reused elsewhere.

## KSJ Libraries

KSJ Libraries represent proven engineering knowledge.

They are intentionally independent of products.

A library should not know whether it is running inside:

- Prototype Box
- Water Node
- Garden Node
- Mushroom Node
- Bird Node
- another future device

It only knows its own responsibility.

Examples:

- KSJ_Storage preserves observations.
- KSJ_Display presents information.
- KSJ_Input produces user events.
- KSJ_Environment describes environmental observations.

## RAMU Nodes

RAMU Nodes are complete devices with stable identities.

A Water Node remains a Water Node.

A Garden Node remains a Garden Node.

A Mushroom Node remains a Mushroom Node.

A node's purpose is stable. Its implementation is expected to evolve.

Nodes compose proven bricks rather than duplicating their internal knowledge.

## Graceful Capability Loss

Hardware should be discoverable.

Software should be composable.

A missing capability must not automatically become total system failure.

Example:

```text
Storage unavailable.
Application continues.
Memory disabled.