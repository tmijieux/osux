cmake_minimum_required(VERSION 2.8)

project(osux)
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)

# basically this allow the installation folder to be moved anywhere
# and the library to still be found correctly



add_definitions(
  -DPKG_DATA_DIR="${CMAKE_INSTALL_PREFIX}/usr/share/${CMAKE_PROJECT_NAME}"
  -DPKG_CONFIG_DIR="/etc/${CMAKE_PROJECT_NAME}"
  -DPYTHON_PATH="${CMAKE_INSTALL_PREFIX}/usr/lib/${CMAKE_PROJECT_NAME}/")


set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (NOT DEFINED CMAKE_ENABLE_C99)
  set(CMAKE_ENABLE_C99 "-std=gnu99")
endif(NOT DEFINED CMAKE_ENABLE_C99)

set(CMAKE_C_FLAGS "${CMAKE_ENABLE_C99} -Wall -Wl,-Xld -Wl,--disable-new-dtags")

enable_testing()

include_directories(include)
include_directories(.)

add_subdirectory(util)
add_subdirectory(python)
add_subdirectory(beatmap)
add_subdirectory(database)
add_subdirectory(editor)
add_subdirectory(mod)
add_subdirectory(replay)
add_subdirectory(taikorank)

# stackoverflow.com/questions/8934295/
#  add-source-in-a-subdirectory-to-a-cmake-project
# http://www.cmake.org/Wiki/CMake/Editors/Emacs

