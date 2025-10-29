add_executable(image.load.filesystem main.cpp)
target_link_libraries(image.load.filesystem PUBLIC engine)
add_test(image.load.filesystem image.load.filesystem ${PROJECT_SOURCE_DIR}/src/engine/image/test/image.load.filesystem/test.png)