# All sources live under src/; headers are included as "core/...", "net/...",
# "gui/...". config.h is generated into this build directory, which meson puts
# on the include path by default.
configure_file(output : 'config.h', configuration : config)

src_inc = include_directories('.')

core_lib = static_library('panel-modeler-core',
  sources : files(
    'core/CsvIO.cpp',
    'core/Equation.cpp',
    'core/Simulation.cpp',
  ),
  include_directories : src_inc,
)
core_dep = declare_dependency(
  link_with : core_lib,
  include_directories : src_inc,
)

cli_deps = [core_dep]

if with_qt
  qt6 = import('qt6')

  # every QObject-derived class needs its header run through moc
  net_moc = qt6.preprocess(
    moc_headers : files(
      'net/ClimateService.h',
      'net/Geocoder.h',
      'net/PanelDatabase.h',
    ),
    include_directories : src_inc,
    dependencies : qt6_dep,
  )

  net_lib = static_library('panel-modeler-net',
    sources : [
      files(
        'net/ClimateService.cpp',
        'net/Geocoder.cpp',
        'net/PanelDatabase.cpp',
      ),
      net_moc,
    ],
    include_directories : src_inc,
    dependencies : qt6_dep,
  )
  net_dep = declare_dependency(
    link_with : net_lib,
    include_directories : src_inc,
    dependencies : qt6_dep,
  )
  cli_deps += net_dep

  # a copy of the panel database next to the binaries so the GUI finds it when
  # run from the build tree (installed builds use the datadir copy)
  fs = import('fs')
  fs.copyfile(files('../data/panels.json'), 'panels.json')

  gui_moc = qt6.preprocess(
    moc_headers : files(
      'gui/ChartWidget.h',
      'gui/MainWindow.h',
      'gui/PanelDatabaseDialog.h',
    ),
    include_directories : src_inc,
    dependencies : qt6_dep,
  )

  executable('panel-modeler-gui',
    sources : [
      files(
        'gui/ChartWidget.cpp',
        'gui/main.cpp',
        'gui/MainWindow.cpp',
        'gui/PanelDatabaseDialog.cpp',
      ),
      gui_moc,
    ],
    dependencies : [core_dep, net_dep],
    install : true,
  )
endif

cli_exe = executable('panel-modeler',
  sources : files('cli/main.cpp'),
  dependencies : cli_deps,
  install : true,
)
