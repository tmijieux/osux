
set(REPLAY_SOURCE
  replay.c
  xz_decomp.c
  )

add_library(replay STATIC ${REPLAY_SOURCE})


add_executable(parse_replay main.c)
target_link_libraries(parse_replay osux_util mod db bm lzma)