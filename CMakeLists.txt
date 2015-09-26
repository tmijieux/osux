cmake_minimum_required(VERSION 3.0.2)

project(osux)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)
set(CMAKE_C_FLAGS "-std=gnu11 -Wall")

set(CMAKE_INSTALL_RPATH "./lib")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN/lib/")

include_directories(. util include)

add_subdirectory(util)
add_subdirectory(embed)
add_subdirectory(replay)
add_subdirectory(beatmap)
add_subdirectory(ranking)
add_subdirectory(editor)
