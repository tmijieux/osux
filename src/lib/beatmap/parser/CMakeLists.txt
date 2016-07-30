

add_subdirectory(parser_py)
add_subdirectory(parser_c)

set(PARSER_SRC parser.c)
add_library(osux_parser OBJECT ${PARSER_SRC})
