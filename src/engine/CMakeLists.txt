cmake_minimum_required(VERSION 3.13.0)

project(engine VERSION 1.0
                  DESCRIPTION "engine"
                  LANGUAGES C)

set(CMAKE_CXXFLAGS "${CMAKE_CFLAGS} -ggdb -Wall -Werror -Wfatal-errors")

add_library(engine)

# message("SDL2 = ${sdl2}")

find_library(glfw glfw)
find_library(sdl2 SDL2)

if (glfw)
  message("Using glfw")
  target_sources(engine PRIVATE interface/src/glfw3.c)
  target_link_libraries(engine PUBLIC ${glfw})
elseif(sdl2)
  message("Using SDL2")
  target_sources(engine PRIVATE src/interface/sdl2.c)
  target_link_libraries(engine PUBLIC ${sdl2})
else()
  message(FATAL_ERROR "No windowing library found")
endif()

# === Embed

add_custom_command(
  OUTPUT embed_default.glb.c embed_forward.vert.c embed_forward.frag.c embed_deferred_geometry.vert.c embed_deferred_light.frag.c
  COMMAND sh ${PROJECT_SOURCE_DIR}/embedsrc.sh
  DEPENDS ${PROJECT_SOURCE_DIR}/embedsrc.sh ${PROJECT_SOURCE_DIR}/embed/default.glb ${PROJECT_SOURCE_DIR}/embed/forward.vert ${PROJECT_SOURCE_DIR}/embed/forward.frag ${PROJECT_SOURCE_DIR}/embed/deferred/geometry.vert ${PROJECT_SOURCE_DIR}/embed/deferred/light.frag
  VERBATIM
)

target_sources(engine PRIVATE
  embed_default.glb.c
  embed_forward.vert.c
  embed_forward.frag.c
  embed_deferred_geometry.vert.c
  embed_deferred_light.frag.c
)

# ===
# Finish

target_include_directories(engine PUBLIC bsp/include)
target_include_directories(engine PUBLIC engine/include)
target_include_directories(engine PUBLIC filesystem/include)
target_include_directories(engine PUBLIC gpu/include)
target_include_directories(engine PUBLIC interface/include)
target_include_directories(engine PUBLIC phys/include)
target_include_directories(engine PUBLIC skel/include)
target_include_directories(engine PUBLIC view/include)
target_include_directories(engine PUBLIC network/include)
target_link_libraries(engine PUBLIC
    glad
    megastructure
)

target_sources(engine PRIVATE
	  filesystem/src/filesystem.c
    bsp/src/alloc.c
    bsp/src/array.c
    bsp/src/build.c
    bsp/src/trace.c
    gpu/src/gpu.c
    interface/src/frame.c
    phys/src/allocator.c
    phys/src/filesystem.c
    phys/src/modify.c
    phys/src/table.c
    phys/src/trace.c
    skel/src/skel.c
    view/src/draw.c
    view/src/manage.c
    network/src/allocator.c
    # engine/src/engine.c
)

# ===
# Testing

include(CTest)
enable_testing()
add_subdirectory(bsp/test/viewer)
add_subdirectory(bsp/test/trace.point)
add_subdirectory(bsp/test/load)
add_subdirectory(phys/test/trace.point)
add_subdirectory(phys/test/trace.ellipsoid)
# add_subdirectory(test/filesystem.add-path)
# add_subdirectory(test/filesystem.compile)
# add_subdirectory(test/interface.compile)
# add_subdirectory(test/view.interface)
add_subdirectory(view/test/mesh-set)
add_subdirectory(view/test/viewer)
