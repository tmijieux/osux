
set(PARSER_PY_SRC  pyfetch.c python_internal.c )

add_library(osux_parser_py MODULE ${PARSER_PY_SRC})
target_link_libraries(osux_parser_py ${PYTHON_LIBRARIES})

install (
  TARGETS osux_parser_py
  LIBRARY DESTINATION lib/osux/
  ARCHIVE DESTINATION lib/osux/
  )

install(
  FILES osux_parse.py
  DESTINATION lib/osux
  )
