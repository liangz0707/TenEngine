# Feature Specification: 009-render-core full

**Feature Branch**: `009-render-core-full`  
**Created**: 2026-01-29  
**Status**: Draft  
**Input**: This feature implements the **full** 009-RenderCore module. Full module spec and public API contract are referenced; scope is the entire module.

## Spec and contract refs (full module)

- **Full module spec**: `docs/module-specs/009-render-core.md`
- **Public API contract**: `specs/_contracts/009-rendercore-public-api.md`
- **Scope**: Full module — all submodules and capabilities (ShaderParams, ResourceDesc, PassProtocol, UniformBuffer).

Implement using only types/APIs from upstream contracts. Upstream: `specs/_contracts/001-core-public-api.md`, `specs/_contracts/008-rhi-public-api.md`.

---

## Clarifications

### Session 2026-01-29

- Q: When ring-buffer slots are exhausted, which behavior does the system use (grow, block, or error)? → A: Block (block or require caller to wait/retry until a slot is released).
- Q: Does CreateLayout mean “create buffer from layout” or “define layout only”? → A: Create buffer from layout (CreateLayout creates a Uniform buffer from a UniformLayout and returns a UniformBufferHandle).
- Q: When a descriptor is requested with unsupported format/size, does the system reject at API call or only document constraints? → A: Reject at API call (return error or invalid handle; do not create resource; caller handles fallback).
- Q: For the same resource declared both read and write in one pass, does RenderCore disallow, allow, or defer to PipelineCore? → A: Defined by PipelineCore (RenderCore does not add restriction; allowed patterns defined by PipelineCore and RHI only).
- Q: When Uniform layout does not align with Shader convention, does the system require runtime validation, optional validation, or document only? → A: Optional validation (system MAY validate at CreateLayout/Bind; on mismatch, return error; validation behavior documented).

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Describe mesh geometry and textures/buffers for RHI (Priority: P1)

A pipeline, mesh, or effect consumer needs to describe vertex/index layout and texture/buffer resources so that the RHI can create them. The system provides VertexFormat, IndexFormat, TextureDesc, and BufferDesc that match RHI creation parameters.

**Why this priority**: Base for all rendering; without ResourceDesc, no geometry or resources can be created.

**Independent Test**: Given logical vertex/index layout and texture/buffer requirements, produce descriptors; verify they are accepted by the RHI creation API (per 008-RHI contract).

**Acceptance Scenarios**:

1. **Given** a defined vertex layout (attributes, strides), **When** the consumer requests a VertexFormat descriptor, **Then** the system returns a descriptor usable for RHI vertex/buffer creation.
2. **Given** a chosen index type (e.g. 16- or 32-bit), **When** the consumer requests an IndexFormat descriptor, **Then** the system returns a descriptor usable for RHI index buffer creation.
3. **Given** texture dimensions and format, **When** the consumer requests a TextureDesc, **Then** the system returns a descriptor usable for RHI texture creation.
4. **Given** buffer size and usage hints, **When** the consumer requests a BufferDesc, **Then** the system returns a descriptor usable for RHI buffer creation.

---

### User Story 2 - Define Uniform layout and resolve offsets (Priority: P1)

A shader or material consumer needs to define Uniform Buffer layout (constant blocks, binding) and resolve member offsets so that shader parameters align with the layout. The system provides DefineLayout and GetOffset; layout aligns with Shader reflection or hand-written layout.

**Why this priority**: Required for Shader/Material; Uniform layout is central to RenderCore.

**Independent Test**: Given a layout definition (or Shader-reflected layout), produce UniformLayout; for each member, GetOffset returns correct offset; layout matches Shader name/type convention.

**Acceptance Scenarios**:

1. **Given** a layout definition (constants, blocks), **When** the consumer calls DefineLayout, **Then** the system returns a UniformLayout usable for Shader binding and RHI.
2. **Given** a UniformLayout and a member name/identifier, **When** the consumer calls GetOffset, **Then** the system returns the correct byte offset for that member.
3. **Given** Shader reflection or hand-written layout, **When** the consumer defines layout, **Then** the resulting layout is consistent with Shader name/type convention (per contract).

---

### User Story 3 - Declare pass read/write and resource lifetime (Priority: P1)

A pipeline author needs to declare which resources a pass reads from and writes to, and resource lifetime, so that PipelineCore can build an RDG-style pass graph. The system provides DeclareRead, DeclareWrite, and ResourceLifetime.

**Why this priority**: Pass protocol is required for PipelineCore, Pipeline, Effects, 2D, Terrain.

**Independent Test**: Given a pass, declare read/write resources and lifetimes; verify declarations are consistent with PipelineCore RDG protocol and RHI constraints.

**Acceptance Scenarios**:

1. **Given** a pass and a resource handle, **When** the consumer declares DeclareRead, **Then** the system records the read declaration (PassResourceDecl) for the pass graph.
2. **Given** a pass and a resource handle, **When** the consumer declares DeclareWrite, **Then** the system records the write declaration for the pass graph.
3. **Given** a declared resource, **When** the consumer sets ResourceLifetime, **Then** the lifetime is recorded and does not violate RHI requirements; Pass graph protocol (PipelineCore) can consume it.

---

### User Story 4 - Create, update, and bind Uniform buffers (Priority: P1)

A shader or material consumer needs to create Uniform buffers from a layout, update them (including multi-frame ring-buffer strategy), and bind them to the RHI for drawing. The system provides CreateLayout (create a Uniform buffer from a UniformLayout and return a UniformBufferHandle), Update, RingBuffer, and Bind.

**Why this priority**: UniformBuffer is required for Shader/Material and multi-Pass rendering.

**Independent Test**: Create a Uniform buffer from a layout; perform Update and (if applicable) RingBuffer; Bind to RHI; verify draws use the correct data per contract.

**Acceptance Scenarios**:

1. **Given** a UniformLayout, **When** the consumer calls CreateLayout, **Then** the system creates a Uniform buffer from that layout and returns a UniformBufferHandle usable for Update and Bind.
2. **Given** a UniformBufferHandle and updated data, **When** the consumer calls Update, **Then** the system updates the buffer content for use in draws.
3. **Given** multi-frame or multi-Pass usage, **When** the consumer uses RingBuffer, **Then** the system supports ring-buffer update strategy without overwriting in-flight data.
4. **Given** a UniformBufferHandle, **When** the consumer calls Bind, **Then** the buffer is bound to RHI for Shader access; binding aligns with Shader module and RHI contract.

---

### Edge Cases

- **Unsupported format or size**: When a descriptor is requested with unsupported format/size, the system MUST reject at API call (return error or invalid handle; do not create resource); caller handles fallback; constraints align with RHI contract.
- **Same resource read and write in one pass**: RenderCore does not add restriction; whether the same resource may be both DeclareRead and DeclareWrite in one pass is defined by PipelineCore and RHI protocol only; DeclareRead/DeclareWrite and ResourceLifetime MUST align with PipelineCore RDG and RHI constraints.
- **Core or RHI not initialized**: System MUST NOT be used before upstream (Core, RHI) is initialized (per contract).
- **Uniform layout vs. Shader mismatch**: Layout MUST align with Shader reflection or hand-written convention; the system MAY validate at CreateLayout or Bind; on mismatch, return error; validation behavior MUST be documented.
- **RingBuffer exhaustion**: When ring-buffer slots are exhausted, the system MUST block or require the caller to wait/retry until a slot is released; no silent overwrite of in-flight data.

---

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST provide VertexFormat, IndexFormat, TextureDesc, and BufferDesc that align with RHI creation parameters (per `specs/_contracts/008-rhi-public-api.md`).
- **FR-002**: System MUST provide DefineLayout and GetOffset; Uniform layout MUST align with Shader reflection or hand-written layout (per contract).
- **FR-003**: System MUST provide DeclareRead, DeclareWrite, and ResourceLifetime; Pass declarations MUST align with PipelineCore RDG protocol and RHI constraints.
- **FR-004**: System MUST provide CreateLayout (create Uniform buffer from UniformLayout, return UniformBufferHandle), Update, RingBuffer, and Bind for Uniform buffers; binding MUST align with Shader module and RHI.
- **FR-005**: System MUST expose only contract types/handles (UniformLayout, VertexFormat, IndexFormat, PassResourceDecl, UniformBufferHandle, TextureDesc, BufferDesc); MUST NOT expose RHI internals.
- **FR-006**: System MUST be used only after Core and RHI are initialized; MUST use only types/APIs declared in upstream contracts.

### Key Entities

- **UniformLayout**: Uniform Buffer layout, constant blocks; aligns with Shader name/type; lifecycle “defined until unload” (per contract).
- **VertexFormat / IndexFormat**: Vertex/index format descriptions for RHI creation; lifecycle “defined until unload.”
- **PassResourceDecl**: Pass input/output resource declarations for PipelineCore RDG; lifecycle “single pass-graph build.”
- **UniformBufferHandle**: Uniform buffer handle; layout, update, ring-buffer, RHI binding; lifecycle “created until explicit release.”
- **TextureDesc / BufferDesc**: Texture and buffer descriptors for RHI creation; managed by caller (per contract).

---

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Consumers can obtain all ResourceDesc types (VertexFormat, IndexFormat, TextureDesc, BufferDesc) and use them for RHI creation across supported configurations.
- **SC-002**: Consumers can define Uniform layouts, resolve offsets via GetOffset, and use layouts for Shader binding and UniformBuffer creation.
- **SC-003**: Consumers can declare pass read/write and ResourceLifetime and have those declarations consumed by PipelineCore (or equivalent) for pass-graph construction without contract violation.
- **SC-004**: Consumers can create Uniform buffers, update them (including RingBuffer), and bind them to RHI for drawing; behavior aligns with Shader and RHI contracts.
- **SC-005**: No use of APIs outside upstream contracts (001-Core, 008-RHI); full contract compliance for 009-RenderCore is verifiable.
- **SC-006**: All four submodules (ShaderParams, ResourceDesc, PassProtocol, UniformBuffer) are implemented and covered by this spec.

---

## Interface Contracts *(multi-agent sync)*

- **This module contract**: `specs/_contracts/009-rendercore-public-api.md` — full module; all types and capabilities in scope.
- **Depends**: `specs/_contracts/001-core-public-api.md`, `specs/_contracts/008-rhi-public-api.md`; implementation uses only declared types/APIs. See `specs/_contracts/000-module-dependency-map.md`.

---

## Dependencies

- **001-Core**: `specs/_contracts/001-core-public-api.md` (memory, math as needed).
- **008-RHI**: `specs/_contracts/008-rhi-public-api.md` (buffer/texture creation, format mapping, binding).
- **010-Shader**: Uniform layout convention (reflection or hand-written); this module provides layout and offsets; no direct implementation dependency beyond contract alignment.
- **019-PipelineCore**: Pass graph (RDG) protocol; this module provides PassResourceDecl and ResourceLifetime; no direct implementation dependency beyond contract alignment.
