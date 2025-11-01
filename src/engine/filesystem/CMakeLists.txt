target_sources(engine PRIVATE src/filesystem.cpp)
target_include_directories(engine PUBLIC include)
add_subdirectory(test/filesystem.base)