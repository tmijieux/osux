
add_subdirectory(parser)

add_library(beatmap OBJECT
  beatmap.c
  color.c
  hitobject.c
  timingpoint.c
  storyboard.c
  $<TARGET_OBJECTS:parser>
  )

# install(DIRECTORY osufiles DESTINATION .)

