# Changelog

## 2.0.0 — Unreleased

This release expands `panel-modeler` from a CSV-only simulator into a command-line and
Qt desktop application.

### Added

- Qt 6 desktop GUI with editable climate, panel, and simulation settings.
- Results table and chart with CSV export.
- NASA POWER climate lookup for annual irradiance and temperature.
- OpenStreetMap Nominatim address geocoding.
- Local and remote JSON panel database support.
- Optional sixth CSV field for the number of identical panels in an array.
- Decimal precision for small temperature derating and degradation values such as `-0.0043`.
- Meson build configuration using Clang, C++23, and mold on Linux.

### Compatibility

- Existing five-value panel CSV rows remain valid and represent one panel.
- Qt is optional at configure time; without Qt, the CSV CLI and simulation core still build.
- Network features require internet access and depend on the usage policies of NASA POWER
  and OpenStreetMap Nominatim.

### Validation

- Clean Qt-enabled Clang build completed successfully.
- CLI example regression test passed.
- Array-count regression test passed.
