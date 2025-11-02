add_executable(json.base main.cpp)
target_link_libraries(json.base PUBLIC engine)
add_test(json.base json.base 
${PROJECT_SOURCE_DIR}/src/engine/json/test/json.base/test1.json
${PROJECT_SOURCE_DIR}/src/engine/json/test/json.base/test2.json
)
