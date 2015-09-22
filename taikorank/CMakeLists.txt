add_executable(taiko_ranking main.c sum.c convert.c print.c taiko_ranking_object.c compute.c density.c reading.c)
target_link_libraries(taiko_ranking parser m heap)
install(TARGETS taiko_ranking DESTINATION .)
install(DIRECTORY aa_maps DESTINATION .)
