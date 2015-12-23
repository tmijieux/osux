
set(heap_src 

	heap.c

	)

add_library(heap SHARED ${heap_src})
install(TARGETS heap DESTINATION lib)

