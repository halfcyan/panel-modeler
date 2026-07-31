project('panel-modeler', 'cpp',
  version : '2.0.0',
  meson_version : '>=0.64.0',
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

# Link with mold on Linux; it's much faster than the default linker. Apple
# Clang's driver does not accept -fuse-ld=mold, so macOS uses the system linker.
if host_machine.system() == 'linux'
  if not find_program('mold', required : false).found()
    error('mold linker not found. Install it and re-run setup.')
  endif
  add_project_link_arguments('-fuse-ld=mold', language : 'cpp')
endif

# Remaining warnings carried over from the old Makefile
add_project_arguments(
  '-Wconversion',
  '-Wdouble-promotion',
  '-Wunreachable-code',
  '-Wshadow',
  language : 'cpp',
)

# Qt6 drives the GUI and the network (climate/geocoding) support. Without it,
# only the plain CSV command-line tool is built (see the 'qt' option).
qt6_dep = dependency('qt6',
                     modules : ['Core', 'Gui', 'Widgets', 'Network'],
                     required : get_option('qt'),
                     include_type : 'system')  # keep Qt's headers from tripping our warnings
with_qt = qt6_dep.found()

# config.h itself is generated inside subdir('src') so it lands in that
# build directory, which is on the include path of every target there.
config = configuration_data()
config.set_quoted('PANEL_MODELER_VERSION', meson.project_version())
config.set_quoted('PANEL_MODELER_DATA_DIR',
                  join_paths(get_option('prefix'), get_option('datadir'), 'panel-modeler'))
if with_qt
  config.set('PANEL_MODELER_WITH_NETWORK', 1)
endif

subdir('src')

install_data('data/panels.json',
             install_dir : join_paths(get_option('datadir'), 'panel-modeler'))
install_data(['README.md', 'LICENSE'],
             install_dir : join_paths(get_option('datadir'), 'doc', 'panel-modeler'))

if host_machine.system() == 'linux'
  install_data('data/panel-modeler.desktop',
               install_dir : join_paths(get_option('datadir'), 'applications'))
endif

# Regression test: CLI output on the example input must match the expected CSV
# byte-for-byte.
python3 = find_program('python3', required : false)
if python3.found()
  test('cli-example',
       python3,
       args : [
         files('tests/compare_output.py'),
         cli_exe.full_path(),
         files('tests/ExamplePanelData.csv'),
         files('tests/ExpectedOutputData.csv'),
       ])
endif

summary({'Qt6 found (GUI + network support)' : with_qt}, section : 'panel-modeler')
