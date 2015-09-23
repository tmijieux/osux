add_library(tp STATIC timing_point.c tp_py.c)
target_link_libraries(tp python2.7)

install(FILES omp_tp.py DESTINATION scripts/python)
