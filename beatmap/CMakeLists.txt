add_subdirectory(hitobject)
add_subdirectory(timingpoint)
add_subdirectory(combocolor)
add_subdirectory(hitsound)

add_library(beatmap SHARED beatmap.c)
target_link_libraries(beatmap hitobject timingpoint combocolor)
install(TARGETS beatmap DESTINATION lib)

add_subdirectory(parser)

# install(DIRECTORY osufiles DESTINATION .)
