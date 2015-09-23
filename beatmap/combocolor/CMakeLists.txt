add_library(coco STATIC combocolor.c)
target_link_libraries(coco python2.7)

install(FILES omp_coco.py DESTINATION scripts/python)