# tinygltf

## 名称与简介

**tinygltf**：Header-only C++ 库，加载 glTF 2.0（.gltf / .glb），含 JSON 解析、base64 解码、贴图/缓冲引用。依赖可选（可仅用 stb_image 加载贴图）。

## 仓库/来源

- **URL**：https://github.com/syoyo/tinygltf  
- **推荐版本**：最新 release 或 v2.x 稳定 tag

## 许可证

MIT。

## CMake 集成

```cmake
include(FetchContent)
FetchContent_Declare(
  tinygltf
  GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
  GIT_TAG        v2.8.0
)
FetchContent_MakeAvailable(tinygltf)
# 使用: tinygltf::tinygltf
```

## 引用方式（自动集成）

- **变量**：`TENENGINE_USE_TINYGLTF=ON`（012-Mesh、013-Resource 选用 tinygltf 时）。  
- **清单**：`tinygltf`。  
- 与 cgltf 二选一或并存：tinygltf 为 C++、功能更全；cgltf 为 C、单头文件更轻。

## 可选配置

- 可关闭 STB 图像加载，改用 libpng/libjpeg 等；按需启用 TINYGLTF_USE_CPP14 等。

## 使用模块

012-Mesh、013-Resource（glTF 模型与贴图引用）；024-Editor（glTF 预览/导入）。
