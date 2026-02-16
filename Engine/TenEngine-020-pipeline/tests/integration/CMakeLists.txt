# 020-Pipeline integration tests
add_executable(te_pipeline_test_flow
  test_render_flow.cpp
)
target_link_libraries(te_pipeline_test_flow PRIVATE te_pipeline te_pipelinecore)
