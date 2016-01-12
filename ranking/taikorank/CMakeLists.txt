
set(tr_src
  main.c
  check_osu_file.c
  mods.c
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
  compute_star/density.c
  compute_star/reading.c
  compute_star/pattern.c
  compute_star/accuracy.c
  compute_star/final_star.c
  )

add_executable(taiko_ranking ${tr_src})
target_link_libraries(taiko_ranking osux m util mysqlclient)

include_directories(. compute_star)

install(TARGETS taiko_ranking DESTINATION .)
##install(DIRECTORY aa_maps DESTINATION .)
