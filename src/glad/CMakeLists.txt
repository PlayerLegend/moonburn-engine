cmake_minimum_required(VERSION 3.13.0)

project(glad VERSION 1.0
                  DESCRIPTION "glad"
                  LANGUAGES C)

set(CMAKE_CFLAGS "${CMAKE_CFLAGS} -g -Wall -Werror -Wfatal-errors")

add_library(glad)
target_include_directories(glad PUBLIC include)
target_sources(glad PRIVATE
	src/glad.c
)

include(CTest)
enable_testing()