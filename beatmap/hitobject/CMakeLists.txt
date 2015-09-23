add_library(hitobject SHARED hit_object.c pyhitobject.c)
target_link_libraries(hitobject python2.7)

install(FILES omp_ho.py DESTINATION scripts/python)
