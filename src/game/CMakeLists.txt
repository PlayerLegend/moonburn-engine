cmake_minimum_required(VERSION 3.13.0)

project(game VERSION 1.0
                  DESCRIPTION "engine"
                  LANGUAGES CXX)

set(CMAKE_CFLAGS "${CMAKE_CFLAGS} -ggdb -Wall -Werror -Wfatal-errors")

add_library(game)

# === Embed

add_custom_command(
  OUTPUT embed_dagger_dagger.glb.c embed_world_world.glb.c
  COMMAND sh ${PROJECT_SOURCE_DIR}/embedsrc.sh
  DEPENDS ${PROJECT_SOURCE_DIR}/embedsrc.sh ${PROJECT_SOURCE_DIR}/embed/dagger/dagger.glb ${PROJECT_SOURCE_DIR}/embed/world/world.glb
  VERBATIM
)

target_sources(game PRIVATE
  embed_dagger_dagger.glb.c
  embed_world_world.glb.c
)

# ===
# Finish

target_link_libraries(game PUBLIC
    glad
    megastructure
    engine
)

target_sources(game PRIVATE
	  src/player.c
	  src/main.c
	  src/input.c
	  src/embed.c
	  src/dagger.c
	  src/world.c
    src/flyprojectile.c
)

# ===
# Testing

include(CTest)
enable_testing()
add_executable(client src/main.c)
target_link_libraries(client game)

# The game takes place in several layers
# - Game logic on the server
# - Physics on client/server
# - Network sync on client/server
