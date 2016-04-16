
set(PYTHON_SOURCE
  python.c python.h)

add_library(pythonembed STATIC ${PYTHON_SOURCE})
