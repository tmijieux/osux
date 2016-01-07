include_directories(.)

#add_library(list STATIC list.c)
add_library(list SHARED list.c)
target_link_libraries(list hashtable)
install(TARGETS list DESTINATION lib)
