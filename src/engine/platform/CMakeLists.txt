target_sources(engine PRIVATE src/platform.cpp src/sdl.cpp)
target_include_directories(engine PUBLIC include)
add_subdirectory(test/platform.base)