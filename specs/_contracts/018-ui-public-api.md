# Contract: 018-UI Module Public API

## Applicable Modules

- **Implementer**: 018-UI (L2; controls, canvas, events)
- **Corresponding Spec**: `docs/module-specs/018-ui.md`
- **Dependencies**: 017-UICore

## Consumers

- 024-Editor

## Implementation Status

**PARTIALLY IMPLEMENTED** - Only ICanvas interface exists in `Engine/TenEngine-018-ui/include/te/ui/Canvas.h`. Widget types, events, and styles are not yet implemented.

## Capabilities List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| ICanvas | Canvas handle; CreateCanvas, AddChild, Layout, Draw; interfaces with UICore Layout/Draw/HitTest | From creation until explicit release |
| WidgetTree | Widget tree; Button, Slider, Text, Image, List, Container, SetStyle | **NOT IMPLEMENTED** - Bound to Canvas |
| EventCallback | Event callbacks; OnClick, OnDrag, OnInput, Bubble, Capture | **NOT IMPLEMENTED** - Registered by caller |
| StyleRef | Theme, color/font/margin, Resource reference | **NOT IMPLEMENTED** - Bound to widget or canvas |

### Capabilities (Provider Guarantees)

| No. | Capability | Description | Status |
|-----|------------|-------------|--------|
| 1 | Canvas | CreateCanvas, AddChild, Layout, Draw; interfaces with UICore Layout/Draw/HitTest | **Implemented** |
| 2 | Widgets | WidgetTree; Button, Slider, Text, Image, List, Container, SetStyle | **NOT IMPLEMENTED** |
| 3 | Events | EventCallback, OnClick, OnDrag, OnInput, Bubble, Capture | **NOT IMPLEMENTED** |
| 4 | Styles | StyleRef, theme, color/font/margin | **NOT IMPLEMENTED** |

## Version / ABI

- Follows Constitution: Public API versioning; breaking changes increment MAJOR.

## Constraints

- Must be used after UICore initialization. If UI rendering layer is drawn by Pipeline, Pipeline consumes UICore output.

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 018-UI contract |
| 2026-02-05 | Unified directory; capabilities list as table |
| 2026-02-22 | Marked implementation status; only ICanvas implemented; widgets/events/styles not yet implemented |
