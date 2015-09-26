#add_library(pyparser MODULE pyparser.c)

add_library(pyparser SHARED
  pyfetch.c
  )

target_link_libraries(pyparser beatmap embed_python)

install(TARGETS pyparser DESTINATION lib)
install(FILES osux_parse.py DESTINATION scripts/python)
