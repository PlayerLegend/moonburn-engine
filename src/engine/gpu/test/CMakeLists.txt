add_executable(gpu.base base.cpp)
target_link_libraries(gpu.base PUBLIC engine)
add_test(gpu.base gpu.base
${PROJECT_SOURCE_DIR}/src/engine/gpu/test/cube.glb
${PROJECT_SOURCE_DIR}/shader/forward/forward.vert
${PROJECT_SOURCE_DIR}/shader/forward/forward.frag
)