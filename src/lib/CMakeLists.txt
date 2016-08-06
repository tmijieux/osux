
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
  OUTPUT beatmap_db.sql.c beatmap_db.sql.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/database/beatmap_db.sql beatmap_db.sql
  COMMAND ${PROJECT_SOURCE_DIR}/scripts/embedder2.py beatmap_db.sql
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/database/beatmap_db.sql
  COMMENT "embedding beatmap schema; generating c code"
  )

add_custom_target(generate_sql_c DEPENDS beatmap_db.sql.c beatmap_db.sql.h)

file(GLOB_RECURSE osux_src "*.c" "*.h")
set(osux_src ${osux_src} beatmap_db.sql.c beatmap_db.sql.h)

add_library(osux STATIC ${osux_src})

target_link_libraries(osux
  ${GLIB_LIBRARIES}
  ${GLIB_GMODULE_LIBRARIES}
  ${GLIB_GIO_LIBRARIES}
  ${GLIB_GOBJECT_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${LIBLZMA_LIBRARIES}
  ${LIBYAML_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  )
add_dependencies(osux generate_sql_c)
add_sanitizers(osux)