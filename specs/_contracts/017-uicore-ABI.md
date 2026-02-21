# 017-UICore Module ABI

- **Contract**: [017-uicore-public-api.md](./017-uicore-public-api.md) (capabilities and type descriptions)
- **This File**: 017-UICore external ABI explicit table.
- **Reference**: Unity Canvas/RectTransform, UE Slate; layout, draw list, hit test, focus.
- **Naming**: Member methods use **PascalCase**; description column provides **complete function signatures**.

## ABI Table

Column definitions: **Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description**

### Layout

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 017-UICore | te::uicore | Size | struct | Size dimensions | te/uicore/Layout.h | Size | `struct Size { float width = 0.f; float height = 0.f; };` |
| 017-UICore | te::uicore | Rect | struct | Rectangle | te/uicore/Layout.h | Rect | `struct Rect { float x = 0.f; float y = 0.f; float width = 0.f; float height = 0.f; };` |
| 017-UICore | te::uicore | ILayoutNode | Abstract Interface | Measure and Arrange | te/uicore/Layout.h | ILayoutNode::Measure, Arrange | `void Measure(Size const& available);` `void Arrange(Rect const& finalRect);` |
| 017-UICore | te::uicore | ILayoutNode | Abstract Interface | Dirty flag | te/uicore/Layout.h | ILayoutNode::SetDirty, IsDirty | `void SetDirty();` `bool IsDirty() const;` |
| 017-UICore | te::uicore | ILayoutNode | Abstract Interface | Desired size | te/uicore/Layout.h | ILayoutNode::GetDesiredSize | `Size GetDesiredSize() const;` |
| 017-UICore | te::uicore | ILayoutNode | Abstract Interface | Arrange rect | te/uicore/Layout.h | ILayoutNode::GetArrangeRect | `Rect GetArrangeRect() const;` |
| 017-UICore | te::uicore | — | Free Function | DPI Scale | te/uicore/Layout.h | GetDPIScale | `float GetDPIScale();` Interface with Application window/DPI |

### Drawing

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 017-UICore | te::uicore | Color | struct | RGBA color | te/uicore/DrawList.h | Color | `struct Color { float r = 1.f, g = 1.f, b = 1.f, a = 1.f; };` |
| 017-UICore | te::uicore | IDrawCommandList | Abstract Interface | Draw rectangle | te/uicore/DrawList.h | IDrawCommandList::DrawRect | `void DrawRect(Rect const& rect, Color const& color);` |
| 017-UICore | te::uicore | IDrawCommandList | Abstract Interface | Draw texture | te/uicore/DrawList.h | IDrawCommandList::DrawTexture | `void DrawTexture(Rect const& rect, void* textureId, Rect const* uv);` |
| 017-UICore | te::uicore | IDrawCommandList | Abstract Interface | Draw text | te/uicore/DrawList.h | IDrawCommandList::DrawText | `void DrawText(Rect const& rect, char const* text, void* font, Color const& color);` |
| 017-UICore | te::uicore | IDrawCommandList | Abstract Interface | Submit | te/uicore/DrawList.h | IDrawCommandList::Submit | `void Submit();` Submit to ImGui backend; does not depend on Pipeline |
| 017-UICore | te::uicore | — | Free Function/Factory | Create draw command list | te/uicore/DrawList.h | CreateDrawCommandList | `IDrawCommandList* CreateDrawCommandList();` Caller does not own; single-frame use |

### Hit Test

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 017-UICore | te::uicore | HitTestResult | struct | Hit test result | te/uicore/HitTest.h | HitTestResult | `struct HitTestResult { ILayoutNode* node = nullptr; float localX = 0.f; float localY = 0.f; bool hit = false; };` |
| 017-UICore | te::uicore | — | Free Function | Hit test | te/uicore/HitTest.h | HitTest | `HitTestResult HitTest(ILayoutNode* root, int x, int y);` Interface with Input event routing |

### Focus Chain

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 017-UICore | te::uicore | IFocusChain | Abstract Interface | Get/Set focus | te/uicore/Focus.h | IFocusChain::GetFocus, SetFocus | `ILayoutNode* GetFocus() const;` `void SetFocus(ILayoutNode* node);` |
| 017-UICore | te::uicore | IFocusChain | Abstract Interface | Route event | te/uicore/Focus.h | IFocusChain::RouteEvent | `bool RouteEvent(te::application::Event const& ev);` Interface with Input events |
| 017-UICore | te::uicore | — | Free Function/Singleton | Get focus chain | te/uicore/Focus.h | GetFocusChain | `IFocusChain* GetFocusChain();` Managed by UICore; caller does not own pointer |

### Font (Optional - Not Implemented)

| Module Name | Namespace | Class Name | Export Form | Interface Description | Header File | Symbol | Description |
|-------------|-----------|------------|-------------|----------------------|-------------|--------|-------------|
| 017-UICore | te::uicore | IFont | Abstract Interface | Load font | te/uicore/Font.h | LoadFont | `IFont* LoadFont(char const* path);` Interface with Resource (optional); returns nullptr on failure |
| 017-UICore | te::uicore | IFont | Abstract Interface | Glyph and atlas | te/uicore/Font.h | IFont::GetGlyph, GetAtlas | `GlyphInfo GetGlyph(uint32_t codePoint) const;` `ITextureResource* GetAtlas() const;` |

*Note: Font interface is documented but not yet implemented in code.*

*Source: Contract capabilities Layout, Draw, HitTest, Focus; Reference Unity Canvas, UE Slate.*

## Change Log

| Date | Change Description |
|------|---------------------|
| T0 Initial | 017-UICore ABI |
| 2026-02-05 | Unified directory format |
| 2026-02-22 | Synchronized with code; updated function signatures to match implementation; Submit uses ImGui backend; Color struct defined; Font marked as not implemented |
