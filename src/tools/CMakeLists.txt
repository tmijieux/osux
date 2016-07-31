
add_executable(parse_replay parse_replay.c)
target_link_libraries(parse_replay osux)

add_executable(makedb makedb.c)
target_link_libraries(makedb osux)

add_executable(readdb readdb.c)
target_link_libraries(readdb osux )

add_executable(taiko_converter taiko_converter.c)
target_link_libraries(taiko_converter osux )

install( 
     TARGETS parse_replay makedb readdb taiko_converter
     RUNTIME DESTINATION bin
#     DESTINATION "${ASSIMP_BIN_INSTALL_DIR}" COMPONENT assimp-dev
) 
