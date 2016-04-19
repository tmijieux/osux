
add_subdirectory(yaml)
include_directories(yaml/include)

set(UTIL_SOURCE
  hash_table.c		hash_table.h 
  heap.c			heap.h
  list.c 			list.h 	list_node.h
  split.c 			split.h
  stack.c 			stack.h
  yaml2.c 			yaml2.h
  uleb128.c 		uleb128.h
  re.c 				re.h
  sum.c 			sum.h
  data.c 			data.h
  md5.c 			md5.h
  dirent.c			dirent.h
  )

add_library(osux_util STATIC ${UTIL_SOURCE})
target_link_libraries(osux_util ${OPENSSL_LIBRARIES} yaml)

install(
  FILES osux_util.py
  DESTINATION lib/osux
  )