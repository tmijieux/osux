add_library(hashtable SHARED hash_table.c)

include_directories(.)

install(TARGETS hashtable DESTINATION lib)
