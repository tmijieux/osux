
set(REPLAY_SOURCE
  replay.c
  xz_decomp.c
  )

add_library(osux_replay STATIC ${REPLAY_SOURCE})

add_executable(parse_replay main.c)
target_link_libraries(
  parse_replay
  
  osux_replay
  osux_util
  osux_mod
  osux_db
  osux_bm
  lzma)