add_library(beatmap SHARED
  beatmap.c
  color.c
  hitobject.c
  timingpoint.c
  storyboard.c
  )

install(TARGETS beatmap DESTINATION lib)

add_subdirectory(parser)

# install(DIRECTORY osufiles DESTINATION .)

