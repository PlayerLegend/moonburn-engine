cmake_minimum_required(VERSION 3.13.0)

project(tagcpp VERSION 1.0
                  DESCRIPTION "moonburn"
                  LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -Wfatal-errors")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CTest)
enable_testing()
add_subdirectory(src/glad)
add_subdirectory(src/engine)