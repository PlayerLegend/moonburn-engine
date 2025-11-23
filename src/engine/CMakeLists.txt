add_library(engine)

find_library(glfw glfw)
find_library(sdl2 SDL2)

# message("SDL2 = ${sdl2}")

# if (glfw)
#   message("Using glfw")
#   target_sources(engine PRIVATE interface/src/glfw3.c)
#   target_link_libraries(engine PUBLIC ${glfw})
# elseif(sdl2)
#   message("Using SDL2")
#   target_sources(engine PRIVATE src/interface/sdl2.c)
#   target_link_libraries(engine PUBLIC ${sdl2})
# else()
#   message(FATAL_ERROR "No windowing library found")
# endif()

# === Embed

# add_custom_command(
#   OUTPUT embed_default.glb.c embed_forward.vert.c embed_forward.frag.c embed_deferred_geometry.vert.c embed_deferred_light.frag.c
#   COMMAND sh ${PROJECT_SOURCE_DIR}/embedsrc.sh
#   DEPENDS ${PROJECT_SOURCE_DIR}/embedsrc.sh ${PROJECT_SOURCE_DIR}/embed/default.glb ${PROJECT_SOURCE_DIR}/embed/forward.vert ${PROJECT_SOURCE_DIR}/embed/forward.frag ${PROJECT_SOURCE_DIR}/embed/deferred/geometry.vert ${PROJECT_SOURCE_DIR}/embed/deferred/light.frag
#   VERBATIM
# )

# target_sources(engine PRIVATE
#   embed_default.glb.c
#   embed_forward.vert.c
#   embed_forward.frag.c
#   embed_deferred_geometry.vert.c
#   embed_deferred_light.frag.c
# )

# ===
# External libs

find_library(libpng png)
if(libpng)
    add_definitions(-DUSE_LIBPNG)
    target_link_libraries(engine PUBLIC ${libpng})
endif()

find_library(libjpeg jpeg)
if(libjpeg)
    add_definitions(-DUSE_LIBJPEG)
    target_link_libraries(engine PUBLIC ${libjpeg})
endif()

find_library(zlib z)
if(zlib)
    target_link_libraries(engine PUBLIC ${zlib})
endif()

# ===
# Finish

target_include_directories(engine PUBLIC include)

target_link_libraries(engine PRIVATE glad)
target_link_libraries(engine PRIVATE ${sdl2})

# ===
# Testing

include(CTest)
enable_testing()
add_subdirectory(basics)
add_subdirectory(json)
add_subdirectory(image)
add_subdirectory(vec)
add_subdirectory(gltf)
add_subdirectory(filesystem)
add_subdirectory(gpu)
add_subdirectory(skel)
add_subdirectory(platform)
add_subdirectory(view3d)
# add_subdirectory(bsp/test/viewer)