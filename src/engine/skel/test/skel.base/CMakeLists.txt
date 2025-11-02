add_executable(skel.base main.cpp)
target_link_libraries(skel.base PUBLIC engine)
add_test(skel.base skel.base
${PROJECT_SOURCE_DIR}/src/engine/skel/test/skel.base/test.glb
)
