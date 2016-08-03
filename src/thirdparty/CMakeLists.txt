

if(NOT YAML_FOUND)
  include_directories(./libyaml/include)
  add_subdirectory(libyaml)
endif(NOT YAML_FOUND)


if(NOT LIBLZMA_FOUND)
  add_subdirectory(xz)
endif(NOT LIBLZMA_FOUND)


if(WIN32)
  set(osux_src ${osux_src} getopt/getopt.c getopt/getopt.h)
endif(WIN32)
  