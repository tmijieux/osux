add_library(timingpoint SHARED timing_point.c pytimingpoint.c)
target_link_libraries(timingpoint python2.7)

install(FILES omp_tp.py DESTINATION scripts/python)
