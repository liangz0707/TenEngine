# Contract: 027-XR Module Public API

## Status: NOT IMPLEMENTED

> **Note**: This module is a placeholder. No implementation exists in the codebase. The contract below defines the intended API for future implementation.

## Applicable Module

- **Implementor**: 027-XR (L4; AR/VR subsystem, HMD, controllers; no downstream)
- **Spec**: `docs/module-specs/027-xr.md`
- **Dependencies**: 001-Core, 007-Subsystems, 006-Input, 020-Pipeline

## Consumers

- None (L4 consumer; provides XR session, frame, submit to XR swap chain to application or runtime; or as Subsystems subsystem)

## Capabilities

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| XRSessionHandle | XR session handle; integrates with Subsystems subsystem and platform XR runtime | Created until session ends |
| XRFrameHandle | Frame handle; viewport, projection, integrates with Pipeline submit and RHI XR swap chain | Per frame |
| Submit Interface | Submits Pipeline output to XR swap chain; consistent with pipeline-to-rci and RHI conventions | Per frame |

### Capabilities (Provider Guarantees)

| ID | Capability | Description |
|----|------------|-------------|
| 1 | Session | XRSessionHandle, integrates with Subsystems and platform XR runtime |
| 2 | Frame | XRFrameHandle, viewport, projection, integrates with Pipeline submit |
| 3 | Submit | Submits Pipeline output to XR swap chain; consistent with RHI conventions |
| 4 | Input | Integrates with 006-Input (XR input) |

## Version / ABI

- Follows Constitution: Public API versioning; breaking changes increment MAJOR.

## Constraints

- Must be used after Core, Subsystems, Input, and Pipeline initialization. Can integrate with 007-Subsystems (XR as subsystem).

## Change Log

| Date | Change |
|------|--------|
| T0 | 027-XR contract created |
| 2026-02-05 | Unified directory; capabilities in table format |
| 2026-02-22 | Marked as NOT IMPLEMENTED (placeholder only); verified no header files exist |
