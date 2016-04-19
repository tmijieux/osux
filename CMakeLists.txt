cmake_minimum_required(VERSION 2.8)

project(osux)
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)

set(PKG_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}")
set(PKG_CONFIG_DIR "${CMAKE_INSTALL_PREFIX}/etc/${CMAKE_PROJECT_NAME}")
set(PKG_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_PROJECT_NAME}")
set(PYTHON_PATH "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_PROJECT_NAME}")

add_definitions(
  -DPKG_DATA_DIR="${PKG_DATA_DIR}"
  -DPKG_CONFIG_DIR="${PKG_CONFIG_DIR}"
  -DPKG_LIB_DIR="${PKG_LIB_DIR}"
  -DPYTHON_PATH="${PYTHON_PATH}")


set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:\$ORIGIN/../lib/osux")

# if (NOT DEFINED CMAKE_ENABLE_C99)
#    set(CMAKE_ENABLE_C99 "-std=gnu99")
# endif(NOT DEFINED CMAKE_ENABLE_C99)

#set(CMAKE_C_FLAGS "${CMAKE_ENABLE_C99} -Wall -Wl,-Xld -Wl,--disable-new-dtags")

enable_testing()

find_package(OpenSSL REQUIRED)


include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(include)
include_directories(.)

add_subdirectory(util)
add_subdirectory(beatmap)
add_subdirectory(database)
add_subdirectory(editor)
add_subdirectory(mod)
add_subdirectory(replay)

add_library(osux STATIC dummy.c)
target_link_libraries(osux osux_bm osux_db osux_python osux_replay osux_util)

add_subdirectory(taikorank)



# stackoverflow.com/questions/8934295/
#  add-source-in-a-subdirectory-to-a-cmake-project
# http://www.cmake.org/Wiki/CMake/Editors/Emacs

