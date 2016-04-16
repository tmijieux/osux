
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

add_library(bm STATIC ${BM_SOURCE})
