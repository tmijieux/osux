
set(map_src
  map.c
  )

add_library(osumap STATIC ${map_src})
target_link_libraries(osumap ho tp coco)

set(parser_src
  parser.c  
  )

add_library(parser STATIC ${parser_src})
target_link_libraries(parser osumap python2.7)

add_executable(reader reader/main.c)
target_link_libraries(reader parser osumap)

install(TARGETS reader DESTINATION .) 
install(DIRECTORY osufiles DESTINATION .)

install(FILES omp.py DESTINATION scripts/python)
