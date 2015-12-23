add_library(sum SHARED sum.c)
include_directories(.)
target_link_libraries(sum heap)
install(TARGETS sum DESTINATION lib)
