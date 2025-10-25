add_executable(image.load.png main.cpp)
target_link_libraries(image.load.png PUBLIC engine)
add_test(image.load.png image.load.png ${PROJECT_SOURCE_DIR}/test/image.load.png/test.png)
