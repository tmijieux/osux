

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()


set(TR_SOURCE
  main.c
  check_osu_file.c
  tr_mods.c
  print.c
  taiko_ranking_map.c
  taiko_ranking_object.c
  taiko_ranking_score.c
  treatment.c
  stats.c
  tr_db.c
  config.c
  options.c
  cst_yaml.c
  vector.c
  linear_fun.c
  spacing_count.c
  freq_counter.c
  density.c
  reading.c
  pattern.c
  accuracy.c
  final_star.c
  )

add_executable(taiko_ranking ${TR_SOURCE})
target_link_libraries(taiko_ranking osux osux_util mysqlclient pthread m)

install( 
     TARGETS taiko_ranking
     RUNTIME DESTINATION bin
#     DESTINATION "${ASSIMP_BIN_INSTALL_DIR}" COMPONENT assimp-dev
)


install(
  DIRECTORY yaml
  DESTINATION etc/osux
  )
