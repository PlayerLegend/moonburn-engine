target_sources(engine PRIVATE src/skel.cpp)
target_include_directories(engine PUBLIC include)
add_subdirectory(test/skel.base)