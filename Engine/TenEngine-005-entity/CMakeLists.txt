cmake_minimum_required(VERSION 3.16)
project(TenEngine-005-entity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

option(TENENGINE_ENTITY_STANDALONE "Build without 001-core, 002-object, 004-scene (use stubs)" OFF)

if(TENENGINE_ENTITY_STANDALONE)
  add_library(te_entity
    src/Entity.cpp
    src/Component.cpp
    src/Transform.cpp
    src/ComponentQuery.cpp
    src/StubScene.cpp
  )
  target_include_directories(te_entity PUBLIC include)
else()
  tenengine_resolve_my_dependencies("005-entity" OUT_DEPS ENTITY_DEPS)
  add_library(te_entity
    src/Entity.cpp
    src/Component.cpp
    src/Transform.cpp
    src/ComponentQuery.cpp
  )
  target_link_libraries(te_entity PRIVATE ${ENTITY_DEPS})
  target_include_directories(te_entity PUBLIC include)
endif()

tenengine_add_module_test(NAME test_entity MODULE_TARGET te_entity SOURCES tests/unit/test_entity.cpp)
tenengine_add_module_test(NAME test_component MODULE_TARGET te_entity SOURCES tests/unit/test_component.cpp)
tenengine_add_module_test(NAME test_transform MODULE_TARGET te_entity SOURCES tests/unit/test_transform.cpp)
tenengine_add_module_test(NAME test_component_query MODULE_TARGET te_entity SOURCES tests/unit/test_component_query.cpp)
tenengine_add_module_test(NAME test_entity_scene MODULE_TARGET te_entity SOURCES tests/integration/test_entity_scene.cpp)
