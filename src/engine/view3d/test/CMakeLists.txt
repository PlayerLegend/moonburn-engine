add_executable(view3.base base.cpp)
target_link_libraries(view3.base PUBLIC engine)
add_test(view3.base view3.base
${PROJECT_SOURCE_DIR}
src/engine/gpu/test/cube.glb
shader/forward/forward
)