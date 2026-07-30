# panel-modeler

`panel-modeler` estimates photovoltaic panel output over time using a PVWatts-style
calculation and exponential panel degradation. It provides both a CSV command-line
interface and a Qt 6 desktop GUI.

The GUI can retrieve annual climate values from a location entered as coordinates or
an address. It also includes a small JSON panel database that can be replaced with a
self-hosted HTTP copy.

## Requirements

- C++23 compiler; Clang is required by the Meson configuration
- Meson 1.0 or newer
- Ninja
- `mold` on Linux; macOS uses the system linker because Apple Clang does not accept
  `-fuse-ld=mold`
- Qt 6 for the GUI and network features

On macOS with Homebrew:

```sh
brew install meson ninja qt
```

Qt is optional at configure time. If it is not available, the CSV CLI and simulation
core still build. To require or disable Qt explicitly, use `-Dqt=enabled` or
`-Dqt=disabled`.

## Build

```sh
meson setup builddir --native-file clang.ini
meson compile -C builddir
```

The executables are created at:

```text
builddir/src/panel-modeler
builddir/src/panel-modeler-gui
```

To start over:

```sh
rm -rf builddir
meson setup builddir --native-file clang.ini
```

## Install

Meson installs both executables, the bundled panel database, and project
Documentation:

```sh
meson install -C builddir
```

Use a different prefix during setup if desired:

```sh
meson setup builddir --native-file clang.ini --prefix="$HOME/.local"
meson compile -C builddir
meson install -C builddir
```

The installed layout is approximately:

```text
<prefix>/bin/panel-modeler
<prefix>/bin/panel-modeler-gui
<prefix>/share/panel-modeler/panels.json
<prefix>/share/doc/panel-modeler/README.md
<prefix>/share/doc/panel-modeler/LICENSE
```

To stage an install without modifying the system:

```sh
DESTDIR=/tmp/panel-modeler-install meson install -C builddir
```

## GUI

Start the GUI with:

```sh
./builddir/src/panel-modeler-gui
```

The GUI supports:

- Address geocoding or direct latitude/longitude entry
- Climate lookup and editable irradiance/temperature values
- Adding panels manually or selecting them from the panel database
- Importing the existing input CSV format
- Running the simulation with a results table and line chart
- Exporting results in the same CSV format as the CLI

The GUI uses one location-wide irradiance and temperature value for all panels. When
importing a CSV whose rows use different climate values, it uses the first row's
values and reports a warning.

## CLI usage

Run a simulation from an input CSV:

```sh
./builddir/src/panel-modeler input.csv output.csv
```

Print help or the version:

```sh
./builddir/src/panel-modeler --help
./builddir/src/panel-modeler --version
```

When Qt is available, look up climate data directly:

```sh
./builddir/src/panel-modeler --climate 33.44,-112.07
./builddir/src/panel-modeler --climate "Phoenix, AZ"
```

The lookup prints a ready-to-edit CSV line. Coordinates use latitude,longitude
order. Addresses are resolved with OpenStreetMap Nominatim, then climate data is
retrieved from NASA POWER.

## Input CSV format

The first line is the number of years to simulate. Each following non-empty line has
five comma-separated values:

1. Reference power in watts
2. Average irradiance in watts per square meter
3. Average temperature in degrees Celsius
4. Temperature derating coefficient of power, as a decimal per degree Celsius
5. Decay rate per year, as a decimal

Example: [`tests/ExamplePanelData.csv`](tests/ExamplePanelData.csv)

```csv
25
400.0, 196.07, 13.5, -0.0035, 0.0045
```

The parser limits years to 100, irradiance to 0–2000 W/m², temperature to -50–60 °C,
temperature derating to -0.05–0.05, and decay to 0–0.10. Values outside those ranges
are clamped with warnings.

## Output and equation

Output contains one header row and one row for each year, starting at Year 0. The
historical formatting is intentionally stable so existing spreadsheets and expected
output files remain compatible.

The estimate is:

```text
P = P_ref * (I / 1000) * (1 + c_T * (T - 25)) * (1 - d)^years
```

where:

- `P_ref` is reference panel power
- `I` is average irradiance in W/m²
- `T` is average temperature in °C
- `c_T` is the temperature derating coefficient
- `d` is the annual decay rate

This is an intentionally simplified model, not a bankable production forecast.

## Network climate data

The network feature uses:

- **NASA POWER climatology API** for `ALLSKY_SFC_SW_DWN` and `T2M`. The API reports
  irradiance in kWh/m²/day; the application converts its annual value to average W/m².
  The climatology endpoint represents the 2001–2020 period in the current API response.
- **OpenStreetMap Nominatim** for address-to-coordinate lookup.

Network requests require internet access. Nominatim has a usage policy and should not
be treated as a bulk geocoding service. For repeated or production use, configure an
appropriate geocoding provider or host your own service. The fetched values are
editable because annual climatology is an approximation and may not match a project's
preferred weather dataset.

## Panel database

The bundled database is [`data/panels.json`](data/panels.json). Values are nominal
examples and should be checked against current manufacturer datasheets before use.
The GUI can load a remote JSON database from a URL, making self-hosting straightforward:

```sh
cd data
python3 -m http.server 8000
```

Then enter:

```text
http://127.0.0.1:8000/panels.json
```

A database document has this shape:

```json
{
  "version": 1,
  "panels": [
    {
      "manufacturer": "Example Solar",
      "model": "Example 400 W",
      "referencePower": 400.0,
      "tempDeratingCoeffPwr": -0.0035,
      "decayRate": 0.005
    }
  ]
}
```

For an installed application, the local database is searched in this order:

1. The path in `PANEL_MODELER_PANEL_DB`
2. `panels.json` next to the executable
3. `<prefix>/share/panel-modeler/panels.json`

The GUI remembers the last remote database URL using `QSettings`.

## Repository layout

```text
src/core/    STL-only simulation and CSV code
src/net/     Qt network services and panel database loading
src/cli/     CSV and climate lookup command-line entry point
src/gui/     Qt Widgets application and custom chart
 data/       bundled panel database
tests/       regression input, expected output, and test runner
```

The core library has no Qt dependency, so it can still be built and tested with
`-Dqt=disabled`. The old raw arrays and heap-allocated streams were replaced with
`std::vector` and RAII streams; `CsvIO` is now stateless and `Simulation::run` owns
the common result-generation path used by both frontends.

## Tests

Run the Meson regression test:

```sh
meson test -C builddir
```

It runs the CLI against `tests/ExamplePanelData.csv` and compares the output
byte-for-byte with `tests/ExpectedOutputData.csv`.

## Zed and clangd

Meson generates `builddir/compile_commands.json`. The project includes
[`.zed/settings.json`](.zed/settings.json), which tells Zed's clangd integration to
use `builddir` as the compilation-database directory. Run Meson setup once before
opening source files so the database exists:

```sh
meson setup builddir --native-file clang.ini
```

The repository also keeps an ignored root symlink named `compile_commands.json` for
other clangd integrations and tools. Recreate it if the symlink is removed:

```sh
ln -sf builddir/compile_commands.json compile_commands.json
```

The optional infrastructure repository contains a rootless Podman service at
`spruce-infra/panel-modeler/` that serves a self-hosted database internally at
`http://panel-modeler-panels/panels.json`. The GUI can use that URL from the Panel
Database dialog.

## Mirroring to GitHub

Tangled is the source of truth for this repository. The Tangled pipeline at
[`.tangled/workflows/mirror.yml`](.tangled/workflows/mirror.yml) mirrors every branch
and tag push to GitHub.

Configure these repository secrets in Tangled:

- `GITHUB_TOKEN`: a GitHub fine-grained personal access token with **Contents: Read and write** permission on the destination repository
- `GITHUB_REPOSITORY`: the GitHub destination in `owner/repository` form

The workflow uses `git push --mirror`, so the GitHub repository should be dedicated to
this mirror and should not receive independent changes. The destination must already
exist on GitHub.
