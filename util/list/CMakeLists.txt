include_directories(.)

#add_library(list STATIC list.c)
add_library(list SHARED list.c)
install(TARGETS list DESTINATION lib)
