# Unit tests for 002-Object (contract: specs/_contracts/002-object-public-api.md).

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # Standalone config: enable testing if not already from parent.
  enable_testing()
endif()

add_executable(te_object_TypeRegistry_test unit/TypeRegistry_test.cpp)
target_link_libraries(te_object_TypeRegistry_test PRIVATE te_object)
add_test(NAME TypeRegistry_test COMMAND te_object_TypeRegistry_test)

add_executable(te_object_Serializer_roundtrip_test unit/Serializer_roundtrip_test.cpp)
target_link_libraries(te_object_Serializer_roundtrip_test PRIVATE te_object)
add_test(NAME Serializer_roundtrip_test COMMAND te_object_Serializer_roundtrip_test)

add_executable(te_object_PropertyBag_test unit/PropertyBag_test.cpp)
target_link_libraries(te_object_PropertyBag_test PRIVATE te_object)
add_test(NAME PropertyBag_test COMMAND te_object_PropertyBag_test)

add_executable(te_object_CreateInstance_test unit/CreateInstance_test.cpp)
target_link_libraries(te_object_CreateInstance_test PRIVATE te_object)
add_test(NAME CreateInstance_test COMMAND te_object_CreateInstance_test)
