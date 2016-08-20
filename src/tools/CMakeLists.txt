
add_executable(parse_replay parse_replay.c)
target_link_libraries(parse_replay osux)
add_sanitizers(parse_replay)

add_subdirectory( dbctl taiko_generator)

add_executable(taiko_converter taiko_converter.c)
target_link_libraries(taiko_converter osux )
add_sanitizers(taiko_converter)

install( 
     TARGETS parse_replay taiko_converter
     RUNTIME DESTINATION bin
)
