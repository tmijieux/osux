add_library(embed_python SHARED python.c)
target_link_libraries(embed_python python2.7)

add_library(module OBJECT module.c)
