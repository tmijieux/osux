add_library(combocolor SHARED combocolor.c)
target_link_libraries(combocolor python2.7)
install(TARGETS combocolor DESTINATION lib)

install(FILES omp_coco.py DESTINATION scripts/python)
