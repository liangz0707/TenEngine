# 027-XR Module ABI

- **Contract**: [027-xr-public-api.md](./027-xr-public-api.md) (capabilities and type descriptions)
- **This file**: 027-XR public ABI explicit table.
- **Status**: NOT IMPLEMENTED

> **Note**: This module is a placeholder. No implementation exists. The ABI below defines the intended symbols for future implementation.

## ABI Table

Column definitions: **Module | Namespace | Class | Export | Interface | Header | Symbol | Description**

### Session

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 027-XR | te::xr | IXRSession | abstract interface | Create/End session | te/xr/XRSession.h | IXRSession::BeginSession, EndSession | `bool BeginSession(XRSessionDesc const& desc);` `void EndSession();` Integrates with Subsystems and platform XR runtime; created until session ends |
| 027-XR | te::xr | -- | free function/factory | Create XR session | te/xr/XRSession.h | CreateXRSession | `IXRSession* CreateXRSession();` Returns nullptr on failure; or get via SubsystemRegistry::GetSubsystem<IXRSession>() |
| 027-XR | te::xr | -- | struct | Session description | te/xr/XRSession.h | XRSessionDesc | Runtime, options; filled by caller |

### Frame

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 027-XR | te::xr | IXRSession | abstract interface | Begin frame | te/xr/XRFrame.h | IXRSession::BeginFrame | `bool BeginFrame();` Returns false if no render this frame; per frame |
| 027-XR | te::xr | IXRSession | abstract interface | End frame | te/xr/XRFrame.h | IXRSession::EndFrame | `void EndFrame();` Per frame |
| 027-XR | te::xr | IXRSession | abstract interface | Get viewport/projection | te/xr/XRFrame.h | IXRSession::GetViewCount, GetViewport, GetProjection | `uint32_t GetViewCount() const;` `void GetViewport(uint32_t viewIndex, Viewport* out) const;` `void GetProjection(uint32_t viewIndex, float nearZ, float farZ, float* outMatrix) const;` Integrates with Pipeline/RHI |
| 027-XR | te::xr | -- | struct | Viewport | te/xr/XRFrame.h | Viewport | x, y, width, height; valid per frame |

### Submit

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 027-XR | te::xr | IXRSession | abstract interface | Submit to XR swap chain | te/xr/XRFrame.h | IXRSession::Submit | `void Submit(ITexture* const* swapImages, uint32_t count);` Submits Pipeline output to XR swap chain; consistent with pipeline-to-rci and RHI conventions; per frame |
| 027-XR | te::xr | IXRSession | abstract interface | Get XR swap chain | te/xr/XRFrame.h | IXRSession::GetSwapChain | `ISwapChain* GetSwapChain(uint32_t viewIndex) const;` Integrates with RHI XR swap chain; for Pipeline render target binding |

### Input (Optional)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 027-XR | te::xr | IXRSession | abstract interface | XR controller/HMD input | te/xr/XRInput.h | IXRSession::GetControllerPose, GetHeadPose | `void GetControllerPose(uint32_t hand, Transform* out) const;` `void GetHeadPose(Transform* out) const;` Integrates with 006-Input module extension (optional) |

## Change Log

| Date | Change |
|------|--------|
| T0 | 027-XR ABI created |
| 2026-02-05 | Unified directory format |
| 2026-02-22 | Marked as NOT IMPLEMENTED; verified no header files exist in Engine/TenEngine-027-xr/ |
