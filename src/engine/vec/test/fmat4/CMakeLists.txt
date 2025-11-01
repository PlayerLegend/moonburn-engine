add_executable(fmat4.test main.cpp)
target_link_libraries(fmat4.test PUBLIC engine)
add_test(fmat4.test fmat4.test)
