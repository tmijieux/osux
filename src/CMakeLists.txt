
set(PKG_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}")
set(PKG_CONFIG_DIR "${CMAKE_INSTALL_PREFIX}/etc/${CMAKE_PROJECT_NAME}")
set(PKG_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_PROJECT_NAME}")
set(PYTHON_PATH "${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_PROJECT_NAME}")

add_definitions(
  -DPKG_DATA_DIR="${PKG_DATA_DIR}"
  -DPKG_CONFIG_DIR="${PKG_CONFIG_DIR}"
  -DPKG_LIB_DIR="${PKG_LIB_DIR}"
  -DPYTHON_PATH="${PYTHON_PATH}"
  -DMODULE_LOAD_PATH="${PKG_LIB_DIR}")


set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:\$ORIGIN/../lib/osux")

set(Python_ADDITIONAL_VERSIONS "2.7")

find_package(PythonLibs REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(GLIB COMPONENTS gmodule gio gobject REQUIRED )
find_package(GTS REQUIRED)
find_package(GTK3 REQUIRED)
find_package(LibLZMA REQUIRED)
find_package(LibYAML REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenMP)

include_directories(include
  ${GLIB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIR})

add_subdirectory(thirdparty)
add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(taikorank)
add_subdirectory(editor)
