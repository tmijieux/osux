add_library(hashtable SHARED hashtable.c)

include_directories(.)

install(TARGETS hashtable DESTINATION lib)
