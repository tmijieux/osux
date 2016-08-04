
add_executable(dbctl dbctl.c cmdline.c)
target_link_libraries(dbctl osux)
add_sanitizers(dbctl)

install( 
     TARGETS dbctl
     RUNTIME DESTINATION bin
#     DESTINATION "${ASSIMP_BIN_INSTALL_DIR}" COMPONENT assimp-dev
) 

