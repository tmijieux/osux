
add_executable(parse_replay parse_replay.c)
target_link_libraries(parse_replay osux)
add_sanitizers(parse_replay)

# add_executable(makedb makedb.c)
# target_link_libraries(makedb osux)

# add_executable(readdb readdb.c)
# target_link_libraries(readdb osux )

add_subdirectory( dbctl )

add_executable(taiko_converter taiko_converter.c)
target_link_libraries(taiko_converter osux )
add_sanitizers(taiko_converter)

install( 
     TARGETS parse_replay taiko_converter
     RUNTIME DESTINATION bin
)
