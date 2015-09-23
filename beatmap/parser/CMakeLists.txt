
add_subdirectory(pyparser)
add_subdirectory(cparser)

add_library(parser SHARED parser.c)
#TEMPORARY:
target_link_libraries(parser pyparser)
# dynamic load one of the two previous module
# need dependancy module / by functionality

add_executable(osuxparser main.c  $<TARGET_OBJECTS:module>)
target_link_libraries(osuxparser parser beatmap )
