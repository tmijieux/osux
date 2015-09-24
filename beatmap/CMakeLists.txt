add_subdirectory(hitobject)
add_subdirectory(timingpoint)
add_subdirectory(combocolor)

add_library(beatmap SHARED beatmap.c)
target_link_libraries(beatmap hitobject timingpoint combocolor)

add_subdirectory(parser)

# install(DIRECTORY osufiles DESTINATION .)
