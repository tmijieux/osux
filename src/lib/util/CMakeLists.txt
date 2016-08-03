
set(UTIL_SOURCE
  hash_table.c
  heap.c      
  list.c      
  split.c     
  stack.c     
  yaml.c     
  uleb128.c   
  sum.c       
  data.c      
  md5.c       
  str.c  )

add_library(osux_util OBJECT ${UTIL_SOURCE})

