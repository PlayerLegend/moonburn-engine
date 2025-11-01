add_executable(gltf.json main.cpp)
target_link_libraries(gltf.json PUBLIC engine)
add_test(gltf.json gltf.json ${PROJECT_SOURCE_DIR}/src/engine/gltf/test/gltf.json/test.glb)