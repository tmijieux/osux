cmake_minimum_required(VERSION 3.0.2)

project(osux)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)
set(CMAKE_C_FLAGS "-std=gnu11 -Wall")

include_directories(. util include)

add_subdirectory(util)

add_subdirectory(hs)
add_subdirectory(ho)
add_subdirectory(tp)
add_subdirectory(sb)
add_subdirectory(replay)
add_subdirectory(combocolor)

add_subdirectory(map)

add_subdirectory(taikorank)
