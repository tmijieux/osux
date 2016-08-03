

set(BM_SOURCE
  beatmap.c
  beatmap_variable.c
  beatmap_old.c
  util.c
  color.c
  hitobject.c
  event.c
  timingpoint.c
  taiko_autoconvert.c )

add_library(osux_bm OBJECT ${BM_SOURCE})
