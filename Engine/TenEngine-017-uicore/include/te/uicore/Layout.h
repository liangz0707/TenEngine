/**
 * @file Layout.h
 * @brief Layout nodes, Size, Rect for UI (contract: specs/_contracts/017-uicore-ABI.md).
 */
#ifndef TE_UICORE_LAYOUT_H
#define TE_UICORE_LAYOUT_H

namespace te {
namespace uicore {

/** Size in pixels (width, height). */
struct Size {
  float width = 0.f;
  float height = 0.f;
};

/** Rectangle (x, y, width, height). */
struct Rect {
  float x = 0.f;
  float y = 0.f;
  float width = 0.f;
  float height = 0.f;
};

/** Layout node interface: Measure, Arrange, dirty flag. */
class ILayoutNode {
public:
  virtual ~ILayoutNode() = default;
  virtual void Measure(Size const& available) = 0;
  virtual void Arrange(Rect const& finalRect) = 0;
  virtual void SetDirty() = 0;
  virtual bool IsDirty() const = 0;
  virtual Size GetDesiredSize() const = 0;
  virtual Rect GetArrangeRect() const = 0;
};

/** Get DPI scale for current display (1.0 = 96 DPI). */
float GetDPIScale();

}  // namespace uicore
}  // namespace te

#endif  // TE_UICORE_LAYOUT_H
