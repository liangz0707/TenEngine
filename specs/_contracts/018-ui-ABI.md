# 018-UI Module ABI

- **Contract**: [018-ui-public-api.md](./018-ui-public-api.md) (capabilities and type descriptions)
- **This File**: 018-UI external ABI explicit table.
- **Reference**: Unity UGUI, UE UMG; canvas, widget tree, events, styles.
- **Naming**: Member methods use **PascalCase**; description column provides **complete function signatures**.

## Implementation Status

**PARTIALLY IMPLEMENTED** - Only ICanvas interface exists. Widget types, events, and styles are not yet implemented.

## ABI Table

Column definitions: **Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description**

### Canvas (Implemented)

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 018-UI | te::ui | ICanvas | Abstract Interface | Add child node | te/ui/Canvas.h | ICanvas::AddChild | `void AddChild(uicore::ILayoutNode* child);` Add to widget tree root |
| 018-UI | te::ui | ICanvas | Abstract Interface | Layout | te/ui/Canvas.h | ICanvas::Layout | `void Layout();` Delegates to UICore Measure/Arrange |
| 018-UI | te::ui | ICanvas | Abstract Interface | Draw | te/ui/Canvas.h | ICanvas::Draw | `void Draw();` Delegates to UICore DrawList |
| 018-UI | te::ui | — | Free Function/Factory | Create canvas | te/ui/Canvas.h | CreateCanvas | `ICanvas* CreateCanvas();` Returns nullptr on failure; interfaces with UICore Layout/Draw/HitTest |

### Widgets (NOT IMPLEMENTED)

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 018-UI | te::ui | IWidget | Abstract Interface | Base widget interface | te/ui/Widget.h | IWidget::GetLayoutNode, AddChild, RemoveChild | **NOT IMPLEMENTED** - Corresponds to UICore ILayoutNode; child node management |
| 018-UI | te::ui | — | Factory/Type | Button | te/ui/Widgets.h | CreateButton | **NOT IMPLEMENTED** - `IWidget* CreateButton(char const* text);` |
| 018-UI | te::ui | — | Factory/Type | Slider | te/ui/Widgets.h | CreateSlider | **NOT IMPLEMENTED** - `IWidget* CreateSlider(float min, float max, float value);` |
| 018-UI | te::ui | — | Factory/Type | Text | te/ui/Widgets.h | CreateText | **NOT IMPLEMENTED** - `IWidget* CreateText(char const* text);` |
| 018-UI | te::ui | — | Factory/Type | Image | te/ui/Widgets.h | CreateImage | **NOT IMPLEMENTED** - `IWidget* CreateImage(ITextureResource* texture);` |
| 018-UI | te::ui | — | Factory/Type | List | te/ui/Widgets.h | CreateList | **NOT IMPLEMENTED** - `IWidget* CreateList();` Scrollable list |
| 018-UI | te::ui | — | Factory/Type | Container | te/ui/Widgets.h | CreateContainer | **NOT IMPLEMENTED** - `IWidget* CreateContainer();` Layout container |
| 018-UI | te::ui | IWidget | Abstract Interface | Set style | te/ui/Widget.h | IWidget::SetStyle | **NOT IMPLEMENTED** - `void SetStyle(StyleRef const& style);` |

### Events (NOT IMPLEMENTED)

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 018-UI | te::ui | IWidget | Abstract Interface | Register event callback | te/ui/Events.h | IWidget::SetOnClick, SetOnDrag, SetOnInput | **NOT IMPLEMENTED** - `void SetOnClick(EventCallback cb);` `void SetOnDrag(DragCallback cb);` `void SetOnInput(InputCallback cb);` |
| 018-UI | te::ui | — | Enum/Type | Event propagation | te/ui/Events.h | EventPhase::Bubble, Capture | **NOT IMPLEMENTED** - Bubble/Capture by implementation convention |

### Style (NOT IMPLEMENTED)

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 018-UI | te::ui | — | struct | Style reference | te/ui/Style.h | StyleRef | **NOT IMPLEMENTED** - Theme, color, font, margin; interface with Resource |
| 018-UI | te::ui | — | Free Function | Set theme | te/ui/Style.h | SetTheme | **NOT IMPLEMENTED** - `void SetTheme(ICanvas* canvas, StyleRef const& theme);` |

*Source: Contract capabilities Widgets, Canvas, Events, Style; Reference Unity UGUI, UE UMG.*

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 018-UI ABI |
| 2026-02-05 | Unified directory format |
| 2026-02-22 | Marked implementation status; ICanvas implemented with AddChild(LayoutNode), Layout(), Draw(); widgets/events/styles not yet implemented |
