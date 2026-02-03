# 009-RenderCore Validation Behavior

Per spec clarification (Session 2026-01-29), the following optional validation behaviors are documented.

## UniformBuffer vs Shader Convention Validation

**Behavior**: Optional validation at `CreateUniformBuffer` / `Bind`.

- The system MAY validate that the `UniformLayout` aligns with the Shader constant block convention (name, type, size, alignment).
- On mismatch:
  - `CreateUniformBuffer` returns an invalid handle.
  - `Bind` may fail silently or log a warning (implementation-defined).

**Rationale**: Strict validation catches mismatches early; optional to support cases where validation is deferred to GPU (debug vs. release builds).

**Configuration**: Implementation may expose a flag or compile-time option to enable/disable validation.

## Reject-at-Call Semantics

All `Create*` functions in ResourceDesc apply reject-at-call semantics:

| Function | Rejection Condition | Behavior |
|----------|---------------------|----------|
| `CreateVertexFormat` | Null attributes, zero count, zero stride, unknown format | Returns invalid `VertexFormat` |
| `CreateIndexFormat` | Unknown index type | Returns invalid `IndexFormat` |
| `CreateTextureDesc` | Zero width/height, unknown format, zero mip levels | Returns invalid `TextureDesc` |
| `CreateBufferDesc` | Zero size | Returns invalid `BufferDesc` |
| `DefineLayout` | Null members, zero count, unknown member type | Returns invalid `UniformLayout` |
| `CreateUniformBuffer` | Invalid layout | Returns invalid `UniformBufferHandle` |

Callers should check `IsValid()` on returned types before use.

## RingBuffer Exhaustion

**Behavior**: Block (caller wait/retry).

- `RingBufferAdvance` returns `false` if all slots are in-flight.
- `RingBufferAllocSlot` returns `UINT32_MAX` if exhausted.
- Caller must wait for GPU to finish with a slot (via `RingBufferReleaseSlot` or frame synchronization) before retrying.
- No silent overwrite of in-flight data.

## Same-Resource Read+Write in One Pass

**Behavior**: Defined by PipelineCore only.

- RenderCore does not add restriction.
- Allowed patterns are defined by PipelineCore RDG and RHI constraints.
