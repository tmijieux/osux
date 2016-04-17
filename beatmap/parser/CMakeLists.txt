

add_library(osux_parser STATIC parser.c)
target_link_libraries(osux_parser dl)

add_library(osux_pyparser STATIC pyfetch.c)
target_link_libraries(osux_pyparser pythonembed)