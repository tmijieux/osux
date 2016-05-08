
set(PYTHON_SOURCE
  python.c python.h)

add_library(pythonembed SHARED ${PYTHON_SOURCE})
target_link_libraries(pythonembed python2.7)

install (
  TARGETS pythonembed
  LIBRARY DESTINATION lib/osux/
  )