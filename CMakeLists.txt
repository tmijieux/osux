cmake_minimum_required(VERSION 2.8)

project(osux C)

set(CMAKE_MODULE_PATH ${osux_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)

set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale/")
include(cmake/GettextTranslate.cmake)

find_package(Sanitizers)
add_definitions(-DDEBUG=1)
add_definitions(-DPACKAGE="osux")
add_definitions(-DLOCALEDIR="${LOCALEDIR}")
add_definitions(-DENABLE_NLS)

add_subdirectory(src)
add_subdirectory(po)

# stackoverflow.com/questions/8934295/
#  add-source-in-a-subdirectory-to-a-cmake-project
# http://www.cmake.org/Wiki/CMake/Editors/Emacs

