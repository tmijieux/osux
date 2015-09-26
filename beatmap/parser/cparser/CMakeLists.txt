
add_library(cparser MODULE fetch.c parse.c re.c)
target_link_libraries(cparser beatmap)
