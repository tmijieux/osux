
add_library(pyparser OBJECT pyfetch.c pyfetch.h)
add_library(cparser OBJECT cparse.c cparse.h cfetch.c cfetch.c)


add_library(parser OBJECT
  parser.c
  parser.h
  $<TARGET_OBJECTS:pyparser>  
  )

add_executable(osuxparser main.c
  $<TARGET_OBJECTS:parser>
  $<TARGET_OBJECTS:beatmap>
  )

