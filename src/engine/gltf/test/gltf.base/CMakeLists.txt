add_executable(gltf.base main.cpp)
target_link_libraries(gltf.base PUBLIC engine)
add_test(gltf.base gltf.base
${PROJECT_SOURCE_DIR}/src/engine/gltf/test/gltf.base/test1.glb
${PROJECT_SOURCE_DIR}/src/engine/gltf/test/gltf.base/test2.glb
)