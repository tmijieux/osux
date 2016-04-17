
set(UTIL_SOURCE
  hash_table.c
  heap.c
  list.c
  split.c
  stack.c
  yaml2.c
  uleb128.c
  re.c
  sum.c
  data.c
  md5.c)

add_library(osux_util STATIC ${UTIL_SOURCE})
target_link_libraries(osux_util ssl crypto yaml)

install(
  FILES osux_util.py
  DESTINATION lib/osux
  )