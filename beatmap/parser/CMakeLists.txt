

add_subdirectory(parser_py)
add_subdirectory(parser_c)

set(PARSER_SRC parser.c parser.h)

add_library(osux_parser STATIC ${PARSER_SRC})
target_link_libraries(osux_parser ${CMAKE_DL_LIBS})
