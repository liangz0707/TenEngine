# Contract: 017-UICore Module Public API

## Applicable Modules

- **Implementer**: 017-UICore (L2; UI layout, drawing, input routing)
- **Corresponding Spec**: `docs/module-specs/017-ui-core.md`
- **Dependencies**: 001-Core, 003-Application, 006-Input

## Consumers

- 018-UI, 024-Editor

## Capabilities List

### Types and Handles (Cross-Boundary)

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| ILayoutNode | Layout tree node; Measure, Arrange, SetDirty, GetDesiredSize, GetArrangeRect | Managed by widget tree |
| IDrawCommandList | Draw command list; DrawRect, DrawTexture, DrawText, Submit | Single frame or single submission cycle |
| HitTestResult | Hit test result; node, localX, localY, hit flag | Single query |
| IFocusChain | Focus chain; GetFocus, SetFocus, RouteEvent | Managed by UICore singleton |
| Size / Rect / Color | Basic types; width/height, rectangle coords, RGBA color | Value types |

### Capabilities (Provider Guarantees)

| No. | Capability | Description |
|-----|------------|-------------|
| 1 | Layout | ILayoutNode, Measure, Arrange, SetDirty, IsDirty, GetDesiredSize, GetArrangeRect, GetDPIScale |
| 2 | Drawing | IDrawCommandList, DrawRect, DrawTexture, DrawText, Submit (to ImGui backend) |
| 3 | Input Routing | HitTest, HitTestResult; IFocusChain focus management and event routing |

## Version / ABI

- Follows Constitution: Public API versioning; breaking changes increment MAJOR.

## Constraints

- Must be used after Core, Application, Input initialization. Interface with 018-UI widget tree and 024-Editor viewport overlay must be explicit.

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 017-UICore contract |
| 2026-02-05 | Unified directory; capabilities list as table |
| 2026-02-22 | Synchronized with code; added ILayoutNode methods, Color struct, Submit method details |
