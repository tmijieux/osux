
set(library_src	split.c)
add_library(string SHARED ${library_src})
target_link_libraries(string list)
install(TARGETS string DESTINATION lib)
