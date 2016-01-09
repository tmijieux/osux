add_subdirectory(scripts)
include_directories(scripts)

set(util_src
	hash_table.c
	hash_table.h
	list.c
	list.h
	split.c
	split.h
	heap.c
	heap.h
	sum.c
	sum.h
	yaml2.c
	yaml2.h
	stack.c
	stack.h
	)

add_library(util SHARED ${util_src})
target_link_libraries(util yaml)
install(TARGETS util DESTINATION lib)
