
add_custom_command(
  OUTPUT _db_sql_data.c
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/db.sql db.sql
  COMMAND ${PROJECT_SOURCE_DIR}/scripts/embedder.py db.sql > _db_sql_data.c
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/db.sql
  COMMENT "generate c code to dump a file in executable"
  )

add_custom_target(generate_sql_c DEPENDS _db_sql_data.c)

set(DB_SOURCE
  _db_sql_data.c
  osux_db.c)
  
add_library(db STATIC ${DB_SOURCE})
add_dependencies(db generate_sql_c)

add_executable(makedb makedb.c)
target_link_libraries(makedb db util bm)

add_executable(readdb readdb.c)
target_link_libraries(readdb db util bm)