target_sources(engine PRIVATE src/image.cpp)
target_include_directories(engine PUBLIC include)
add_subdirectory(test/image.load.png)
add_subdirectory(test/image.load.filesystem)