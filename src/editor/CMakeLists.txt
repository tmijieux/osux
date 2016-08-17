

file(GLOB_RECURSE ui_src  "*.ui")
file(GLOB editor_src "*.c" "*.h")

add_custom_command(
  OUTPUT resources.c
  COMMAND glib-compile-resources ${CMAKE_CURRENT_SOURCE_DIR}/editor.gresources.xml --target=resources.c --sourcedir=${CMAKE_CURRENT_SOURCE_DIR} --generate-source
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/editor.gresources.xml ${ui_src}
  COMMENT "embedded resources source"
  )

add_custom_command(
  OUTPUT resources.h
  COMMAND glib-compile-resources ${CMAKE_CURRENT_SOURCE_DIR}/editor.gresources.xml --target=resources.h --sourcedir=${CMAKE_CURRENT_SOURCE_DIR} --generate-header
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/editor.gresources.xml ${ui_src}
  COMMENT "embedded resources header"
  )

add_custom_target(editor_resources DEPENDS resources.c resources.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${GTK3_INCLUDE_DIRS})

message(STATUS "GTK include dir:" ${GTK_INCLUDE_DIR})
message(STATUS "GTK lib:" ${GTK_LIBRARIES})

set(editor_src
  ${editor_src}
  ${CMAKE_CURRENT_BINARY_DIR}/resources.c
  ${CMAKE_CURRENT_BINARY_DIR}/resources.h
  )

add_executable(osux_editor ${editor_src})
target_link_libraries(osux_editor
  ${GLIB_GIO_LIBRARIES}
  ${GLIB_GMODULE_LIBRARIES}
  ${GLIB_GOBJECT_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GTK3_LIBRARIES}
  )

add_dependencies(osux_editor editor_resources)


add_sanitizers(osux_editor)

install(
  TARGETS osux_editor
  RUNTIME DESTINATION bin
  )
