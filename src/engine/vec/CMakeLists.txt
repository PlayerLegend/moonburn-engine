target_sources(engine PRIVATE src/vec.cpp)
target_include_directories(engine PUBLIC include)
# add_subdirectory(test/vec.base)
add_subdirectory(test/fmat4)