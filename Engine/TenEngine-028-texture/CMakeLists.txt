# 028-Texture module (contract: specs/_contracts/028-texture-ABI.md). CPU-only: Load/Save/Import, no GPU.
# Dependencies: 001-core, 002-object, 009-rendercore, 013-resource.
cmake_minimum_required(VERSION 3.16)
project(TenEngine-028-texture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")

if(NOT DEFINED TENENGINE_ROOT)
  get_filename_component(_CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
  get_filename_component(_ENGINE_DIR "${_CURRENT_DIR}" DIRECTORY)
  set(TENENGINE_ROOT "${_ENGINE_DIR}")
endif()

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

set(TENENGINE_001_CORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-001-core")
set(TENENGINE_002_OBJECT_DIR "${TENENGINE_ROOT}/Engine/TenEngine-002-object")
set(TENENGINE_009_RENDERCORE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-009-render-core")
set(TENENGINE_013_RESOURCE_DIR "${TENENGINE_ROOT}/Engine/TenEngine-013-resource")

tenengine_resolve_dependency(013-resource RES_TARGET)
if(NOT TARGET te_core)
  tenengine_resolve_dependency(001-core CORE_TARGET)
else()
  set(CORE_TARGET te_core)
endif()
if(NOT TARGET te_object)
  tenengine_resolve_dependency(002-object OBJECT_TARGET)
else()
  set(OBJECT_TARGET te_object)
endif()
tenengine_resolve_dependency(009-rendercore RENDERCORE_TARGET)

# Optional image import libraries (docs/third_party)
option(TENENGINE_USE_LIBPNG "Use libpng for PNG import" OFF)
option(TENENGINE_USE_JPEG_TURBO "Use libjpeg-turbo for JPEG import" OFF)
option(TENENGINE_USE_LIBWEBP "Use libwebp for WebP import" OFF)

include(FetchContent)

# stb (header-only) for image import
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
)
FetchContent_Populate(stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

# libpng and libjpeg-turbo: use find_package (vcpkg or system); libwebp: FetchContent.
if(TENENGINE_USE_LIBPNG)
  find_package(PNG REQUIRED)
endif()

# libjpeg-turbo does not support add_subdirectory; use find_package (vcpkg or system).
if(TENENGINE_USE_JPEG_TURBO)
  find_package(JPEG REQUIRED)
endif()

if(TENENGINE_USE_LIBWEBP)
  FetchContent_Declare(
    libwebp
    GIT_REPOSITORY https://github.com/webmproject/libwebp.git
    GIT_TAG        v1.4.0
  )
  FetchContent_MakeAvailable(libwebp)
endif()

set(TEXTURE_SOURCES
  src/Texture.cpp
  src/TextureFactory.cpp
  src/TextureModule.cpp
  src/TextureResource.cpp
  src/TextureSerialize.cpp
  src/TextureModuleInit.cpp
  src/detail/texture_data.cpp
  src/import/ImageImporterRegistry.cpp
  src/import/StbImageImporter.cpp
)
if(TENENGINE_USE_LIBPNG)
  list(APPEND TEXTURE_SOURCES src/import/LibPngImporter.cpp)
endif()
if(TENENGINE_USE_JPEG_TURBO)
  list(APPEND TEXTURE_SOURCES src/import/LibJpegTurboImporter.cpp)
endif()
if(TENENGINE_USE_LIBWEBP)
  list(APPEND TEXTURE_SOURCES src/import/LibWebPImporter.cpp)
endif()

set(TEXTURE_HEADERS
  include/te/texture/Texture.h
  include/te/texture/TextureAssetDesc.h
  include/te/texture/TextureFactory.h
  include/te/texture/TextureModule.h
  include/te/texture/TextureModuleInit.h
  include/te/texture/TextureResource.h
  include/te/texture/TextureSerialize.h
  include/te/texture/detail/texture_data.hpp
  include/te/texture/import/ImageImporter.h
  include/te/texture/import/ImageImporterRegistry.h
  include/te/texture/import/LibJpegTurboImporter.h
  include/te/texture/import/LibPngImporter.h
  include/te/texture/import/LibWebPImporter.h
  include/te/texture/import/StbImageImporter.h
)

add_library(te_texture STATIC ${TEXTURE_SOURCES} ${TEXTURE_HEADERS})
target_include_directories(te_texture
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${stb_SOURCE_DIR}
)
target_link_libraries(te_texture
  PUBLIC ${RENDERCORE_TARGET} ${RES_TARGET}
  PRIVATE ${CORE_TARGET} stb
)
if(TENENGINE_USE_LIBPNG)
  target_link_libraries(te_texture PUBLIC PNG::PNG)
  target_compile_definitions(te_texture PRIVATE TENENGINE_USE_LIBPNG)
endif()
if(TENENGINE_USE_JPEG_TURBO)
  target_link_libraries(te_texture PUBLIC JPEG::JPEG)
  target_compile_definitions(te_texture PRIVATE TENENGINE_USE_JPEG_TURBO)
endif()
if(TENENGINE_USE_LIBWEBP)
  target_include_directories(te_texture PUBLIC ${libwebp_SOURCE_DIR}/src)
  target_link_libraries(te_texture PUBLIC webp)
  target_compile_definitions(te_texture PRIVATE TENENGINE_USE_LIBWEBP)
endif()
if(OBJECT_TARGET AND TARGET ${OBJECT_TARGET})
  target_link_libraries(te_texture PUBLIC ${OBJECT_TARGET})
endif()

source_group("Header Files" FILES ${TEXTURE_HEADERS})
source_group("Header Files\\Detail" FILES include/te/texture/detail/texture_data.hpp)
source_group("Header Files\\Import" FILES
  include/te/texture/import/ImageImporter.h
  include/te/texture/import/ImageImporterRegistry.h
  include/te/texture/import/LibJpegTurboImporter.h
  include/te/texture/import/LibPngImporter.h
  include/te/texture/import/LibWebPImporter.h
  include/te/texture/import/StbImageImporter.h
)
set_target_properties(te_texture PROPERTIES PUBLIC_HEADER "${TEXTURE_HEADERS}")

# 作为上游被引用时不加入测试
if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
