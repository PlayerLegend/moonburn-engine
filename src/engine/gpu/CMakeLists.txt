target_sources(engine PRIVATE src/gpu.cpp)
target_include_directories(engine PUBLIC include)
add_subdirectory(test)