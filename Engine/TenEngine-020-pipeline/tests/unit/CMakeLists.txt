# 020-Pipeline unit tests
add_executable(te_pipeline_test_abi
  test_abi_contract.cpp
)
target_link_libraries(te_pipeline_test_abi PRIVATE te_pipeline)
