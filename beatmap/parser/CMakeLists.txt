

add_library(osux_parser STATIC parser.c)
target_link_libraries(osux_parser dl)

add_library(osux_pyparser MODULE pyfetch.c )
target_link_libraries(
  osux_pyparser
  "-Wl,--whole-archive"
  pythonembed
  "-Wl,--no-whole-archive")

install(
  TARGETS osux_pyparser
  LIBRARY DESTINATION lib/osux/
  )

install(
  FILES osux_parse.py
  DESTINATION lib/osux
  )