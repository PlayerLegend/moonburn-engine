add_executable(platform.base main.cpp)
target_link_libraries(platform.base PUBLIC engine)
add_test(platform.base platform.base)
