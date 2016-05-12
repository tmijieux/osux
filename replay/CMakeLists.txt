
set(REPLAY_SOURCE
  replay.c
  xz_decomp.c
  )
  
find_package(LibLZMA REQUIRED)

add_library(osux_replay STATIC ${REPLAY_SOURCE})
target_link_libraries(osux_replay ${LIBLZMA_LIBRARIES})

add_executable(parse_replay main.c)

target_link_libraries(parse_replay 
  osux_replay
  osux_mod
  osux_db
  osux_bm
  osux_util
  )