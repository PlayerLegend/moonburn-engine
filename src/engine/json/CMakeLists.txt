target_sources(engine PRIVATE src/json.cpp)
target_include_directories(engine PUBLIC include)
add_subdirectory(test/json.base)