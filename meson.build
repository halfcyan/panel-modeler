project('panel-modeler', 'cpp',
  version : '1.0.0',
  meson_version : '>=0.56.0',
  default_options : [
    'cpp_std=c++23',
    'warning_level=3',  # -Wall -Wextra -Wpedantic
  ],
)

# Remaining warnings carried over from the old Makefile
add_project_arguments(
  '-Wconversion',
  '-Wdouble-promotion',
  '-Wunreachable-code',
  '-Wshadow',
  language : 'cpp',
)

executable('panel-modeler',
  sources : [
    'main.cpp',
    'Equation.cpp',
    'FileIO.cpp',
    'IOProcessor.cpp',
    'Simulation.cpp',
  ],
  install : false,
)
