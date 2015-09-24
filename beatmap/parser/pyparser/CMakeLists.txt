#add_library(pyparser MODULE pyparser.c)

add_library(pyparser SHARED pyparser.c)
# temporary --> custom dynamic loader

target_link_libraries(pyparser beatmap embed_python)

install(FILES omp.py DESTINATION scripts/python)
