set(replay_src
	replay.c
	replay.h
	xz_decomp.c
	xz_decomp.h
	)

add_library(replay SHARED ${replay_src})
target_link_libraries(replay mod lzma string)
install(TARGETS replay DESTINATION lib)


