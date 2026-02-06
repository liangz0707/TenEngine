# TenEngine root: all code lives under Engine/<module>/ (contract-first, specs/_contracts/).
cmake_minimum_required(VERSION 3.16)
project(TenEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Modules: each has Engine/TenEngine-NNN-name/{src,include,tests,cmake,CMakeLists.txt}
# 仅启用已适配单仓的模块；其余 Engine/* 已从 T0 分支同步，启用前需改其 CMake（去重复 target、可选依赖等）。
add_subdirectory(Engine/TenEngine-001-core)
add_subdirectory(Engine/TenEngine-008-rhi)
# add_subdirectory(Engine/TenEngine-002-object)
# add_subdirectory(Engine/TenEngine-004-scene)
# add_subdirectory(Engine/TenEngine-005-entity)
# add_subdirectory(Engine/TenEngine-006-input)
# add_subdirectory(Engine/TenEngine-007-subsystems)
# add_subdirectory(Engine/TenEngine-009-render-core)
# add_subdirectory(Engine/TenEngine-010-shader)
# add_subdirectory(Engine/TenEngine-012-mesh)
# add_subdirectory(Engine/TenEngine-013-resource)
# add_subdirectory(Engine/TenEngine-019-pipeline-core)
