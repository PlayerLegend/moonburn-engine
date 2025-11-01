add_executable(filesystem.base main.cpp)
target_link_libraries(filesystem.base PUBLIC engine)
add_test(filesystem.base filesystem.base)