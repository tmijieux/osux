set(yaml2_src yaml2.c stack.c)

add_library(yaml2 SHARED ${yaml2_src})
target_link_libraries(yaml2 hashtable list yaml)

include_directories(.)

install(TARGETS yaml2 DESTINATION lib)
