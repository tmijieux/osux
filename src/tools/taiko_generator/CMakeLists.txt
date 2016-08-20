
add_executable(taiko_generator taiko_generator.c cmdline.c)
target_link_libraries(taiko_generator osux)
add_sanitizers(taiko_generator)

install( 
     TARGETS taiko_generator
     RUNTIME DESTINATION bin
#     DESTINATION "${ASSIMP_BIN_INSTALL_DIR}" COMPONENT assimp-dev
) 

