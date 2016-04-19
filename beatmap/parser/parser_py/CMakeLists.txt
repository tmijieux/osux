

set(PARSER_PY_SRC 
	pyfetch.c 		pyfetch.h
	python.c 		python.h
	)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
	
add_library(osux_parser_py MODULE ${PARSER_PY_SRC})
target_link_libraries(osux_parser_py ${PYTHON_LIBRARIES})

install (
   TARGETS osux_parser_py
   LIBRARY DESTINATION lib/osux/
   ARCHIVE DESTINATION lib/osux/
)

install(
  FILES osux_parse.py
  DESTINATION lib/osux
)
