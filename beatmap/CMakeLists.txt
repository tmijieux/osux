

add_subdirectory(parser)

set(BM_SOURCE
  beatmap.c
  beatmap.h
  color.c
  color.h
  hitobject.c
  hitobject.h
  hitsound.h
  storyboard.c
  storyboard.h
  timingpoint.c
  timingpoint.h )

add_library(osux_bm STATIC ${BM_SOURCE})
target_link_libraries(osux_bm osux_parser)
