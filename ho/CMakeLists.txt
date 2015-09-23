add_library(ho STATIC hit_object.c ho_py.c)
target_link_libraries(ho python2.7)

install(FILES omp_ho.py DESTINATION scripts/python)
