

add_subdirectory(parser)

set(BM_SOURCE
  beatmap.c
  color.c
  hitobject.c
  storyboard.c
  timingpoint.c
  taiko_autoconvert.c )

add_library(osux_bm OBJECT ${BM_SOURCE})
