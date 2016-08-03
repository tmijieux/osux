


add_custom_command(
  OUTPUT beatmap_db.sql.c beatmap_db.sql.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/beatmap_db.sql beatmap_db.sql
  COMMAND ${PROJECT_SOURCE_DIR}/scripts/embedder2.py beatmap_db.sql
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/beatmap_db.sql
  COMMENT "embedding beatmap schema; generating c code"
  )

add_custom_target(generate_sql_c DEPENDS beatmap_db.sql.c beatmap_db.sql.h)
set(DB_SOURCE beatmap_db.sql.c database.c beatmap_database.c)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(osux_db OBJECT ${DB_SOURCE})
add_dependencies(osux_db generate_sql_c)
