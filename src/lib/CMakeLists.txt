

add_subdirectory(util)
add_subdirectory(beatmap)
add_subdirectory(database)
add_subdirectory(mod)
add_subdirectory(replay)

add_library(osux STATIC
  $<TARGET_OBJECTS:osux_util>
  $<TARGET_OBJECTS:osux_bm>
  $<TARGET_OBJECTS:osux_db>
  $<TARGET_OBJECTS:osux_replay>
  $<TARGET_OBJECTS:osux_mod>)

target_link_libraries(osux
  ${GLIB_LIBRARIES}
  ${GLIB_GMODULE_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${LIBLZMA_LIBRARIES}
  ${LIBYAML_LIBRARIES}
  ${SQLITE3_LIBRARIES}

  )
