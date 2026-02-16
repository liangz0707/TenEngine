# 020-Pipeline unit tests
add_executable(te_pipeline_test_abi
  test_abi_contract.cpp
)
target_link_libraries(te_pipeline_test_abi PRIVATE te_pipeline)

add_executable(te_pipeline_test_render
  test_render_pipeline.cpp
)
target_link_libraries(te_pipeline_test_render PRIVATE te_pipeline te_pipelinecore)
