add_library(embed_python SHARED python.c)
target_link_libraries(embed_python python2.7)
install(TARGETS embed_python DESTINATION lib)

add_library(module OBJECT module.c)
