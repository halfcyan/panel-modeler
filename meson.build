project('panel-modeler', 'cpp',
  version : '1.0.0',
  meson_version : '>=0.56.0',
  default_options : [
    'cpp_std=c++23',
    'warning_level=3',  # -Wall -Wextra -Wpedantic
  ],
)

# This project was written for Clang; refuse to configure with anything else.
cpp_compiler = meson.get_compiler('cpp')
if cpp_compiler.get_id() != 'clang'
  error('panel-modeler must be built with Clang (clang++). ' +
        'Set up with: meson setup builddir --native-file clang.ini')
endif

# Link with mold by default; it's much faster than the default linker.
if not find_program('mold', required : false).found()
  error('mold linker not found. Install it (e.g. brew install mold) and re-run setup.')
endif
add_project_link_arguments('-fuse-ld=mold', language : 'cpp')

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
