
set(tr_src
  main.c
  check_osu_file.c
  mods.c
  print.c
  taiko_ranking_map.c
  taiko_ranking_object.c
  treatment.c
  stats.c
  compute_star/density.c
  compute_star/reading.c
  compute_star/pattern.c
  compute_star/accuracy.c
  compute_star/final_star.c
  $<TARGET_OBJECTS:module>
  )

add_executable(taiko_ranking ${tr_src})

target_link_libraries(taiko_ranking parser m heap hashtable sum yaml)

add_subdirectory(sum)

include_directories(. sum compute_star)

install(TARGETS taiko_ranking DESTINATION .)
##install(DIRECTORY aa_maps DESTINATION .)
