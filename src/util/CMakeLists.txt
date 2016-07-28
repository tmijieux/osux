
set(UTIL_SOURCE
  hash_table.c	      hash_table.h 
  heap.c              heap.h
  list.c              list.h          list_node.h
  split.c             split.h
  stack.c             stack.h
  yaml2.c             yaml2.h
  uleb128.c           uleb128.h
  re.c                re.h
  sum.c               sum.h
  data.c              data.h
  md5.c               md5.h
  dirent_win32.c      dirent_win32.h
  sqlite3.c
  string2.c           string2.h
  )

find_package(Threads REQUIRED)
add_library(osux_util STATIC ${UTIL_SOURCE})
target_link_libraries(
    osux_util ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LIBYAML_LIBRARIES})

install(
  FILES osux_util.py
  DESTINATION lib/osux
  )
