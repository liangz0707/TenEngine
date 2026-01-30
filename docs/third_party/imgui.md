# Dear ImGui（imgui）

## 名称与简介

**Dear ImGui**：即时模式 GUI 库，用于编辑器、调试 UI、工具界面。024-Editor、025-Tools 及调试绘制可依赖。

## 仓库/来源

- **URL**：https://github.com/ocornut/imgui  
- **推荐版本**：`v1.90` 或当前 docking 分支 tag（若使用 docking）

## 许可证

MIT。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90
)
FetchContent_MakeAvailable(imgui)
# 使用: imgui::imgui（需后端：如 imgui_impl_win32 + imgui_impl_dx12/vulkan，可同仓或单独集成）
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_IMGUI=ON`（Editor/Tools 模块启用时）。  
- **清单**：`imgui`。  
- 后端（窗口+图形 API）由 024-Editor 或应用层在 RHI 之上对接，不强制写在第三方文档内。

## 可选配置

- 可选 docking 分支以支持停靠窗口。  
- 仅头文件 + 若干 .cpp，可静态链接。

## 使用模块

024-Editor、025-Tools、调试/开发工具 UI。
