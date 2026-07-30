# Photovoltaic Decay Modeling through PVWatts

A C++ command-line program that predicts the future power output of solar panels based on their
specifications (reference power, temperature derating coefficient, and decay rate). Given a
location's average irradiance and temperature over a year, it calculates the expected power output
a given number of years into the future.

There is no direct user interaction; the program takes two command-line arguments — an input file
and an output file, both in CSV format so they can be opened by Excel or a similar application.
The input file contains the panel specifications, and the output file contains one row per year
with the expected power output of each panel over that timeframe.

## Building

This project builds with [Meson](https://mesonbuild.com/) + [Ninja](https://ninja-build.org/) and
uses Clang by default via the `clang.ini` native file. It targets C++23.

```sh
meson setup builddir --native-file clang.ini
meson compile -C builddir
```

This produces an executable named `panel-modeler` in `builddir/`. To wipe the build directory:

```sh
rm -rf builddir
```

Clang and the [mold](https://github.com/rui314/mold) linker are required — `meson.build`
refuses to configure if the C++ compiler is anything other than Clang or if mold isn't
installed, so always set up with the native file (or `CXX=clang++`). On macOS/Linux with
Homebrew: `brew install llvm mold`.

## Usage

```sh
./builddir/panel-modeler inputFile outputFile
```

- `inputFile` — relative path to the input file in CSV format
- `outputFile` — relative path to the output file

Run `./builddir/panel-modeler` with no arguments (or `./builddir/panel-modeler help`) to print
usage and input formatting help.

## Input Format

The first line of the input file is the number of years to simulate (the same for all panels).
Each subsequent line describes one panel with five comma-separated decimal values, in order:

1. Reference power
2. Average irradiance in watts per square meter
3. Average temperature in degrees Celsius
4. Temperature derating coefficient of power
5. Decay rate per year of the panel

All of these parameters should be available from your solar panel manufacturer. Irradiance and
temperature data for a location can be gathered with the
[SAM tool](https://sam.nrel.gov/) from the National Renewable Energy Lab.

Example (`ExamplePanelData.csv`):

```csv
25
100.0, 196.07, 13.5, -0.0035, 0.0045
100.0, 196.07, 13.5, -0.0035, 0.0045
200.0, 196.07, 13.5, -0.0035, 0.0050
550.0, 247.94, 24.9, -0.0030, 0.0040
550.0, 247.94, 24.9, -0.0030, 0.0040
```

## Output Format

The output CSV has a header row, one column per panel (matching the input rows), and one row per
year starting at Year 0. It can be read by Excel or a similar application to create graphs.

Example (`ExpectedOutputData.csv`, abbreviated):

```csv
      Year,   Panel 1,   Panel 2,   Panel 3,   Panel 4,   Panel 5
    Year 0,   20.3962,   20.3962,   40.7924,  136.4079,  136.4079
    Year 1,   20.3044,   20.3044,   40.5884,  135.8623,  135.8623
    ...
   Year 25,   18.2213,   18.2213,   35.9878,  123.4022,  123.4022
```

## How It Works

The expected power output is calculated with the formula:

```
P = P_ref * (1 + T * T_derate) * (1 - decay_rate) ^ years
```

where `P` is the expected power output, `P_ref` is the reference power, `T` is the average
temperature, `T_derate` is the temperature derating coefficient, `decay_rate` is the decay rate
per year, and `years` is the number of years into the future. This is the standard PVWatts formula
combined with an exponential decay rate formula. More complicated decay formulae exist and are
more accurate, but this approximation is close enough for this project and much easier to
implement.

## Architecture

The program is organized into a few custom classes:

- **`IOProcessor`** — the most complex class. It has functions for reading and writing CSV files,
  plus helper functions for parsing strings and formatting output. It cleans up `main` and makes
  file reading/writing easier.
- **`FileIO`** — a simple wrapper around file streams for easier reading and writing of files.
  `FileIO` and `IOProcessor` work closely together, but file stream handling is kept separate
  from data processing.
- **`Simulation`** — runs the equations that calculate the expected power output for each panel
  over the specified number of years. It is deliberately separate from `Equation` so it can be
  extended with more simulations in the future, with the necessary equations living in the
  equation class. `Simulation` uses `Equation`, but is more complex.
- **`Equation`** — static member functions implementing the expected-power-output equations based
  on the PVWatts formula and the decay formula.
- **`PanelData`** — a simple struct holding the specifications for each panel.

`main.cpp` includes error handling and user interaction, including a help function that shows how
to format the CSV for this program — useful for anyone running it without the author controlling
how they use it.

## Data Structures

- A single array of `PanelData` structs holds the specifications for each panel.
- A 2D array of doubles holds the expected power output for each panel over the specified number
  of years. It is indexed by year and panel number, making it easy to access the expected power
  output for any given year and panel — critical for the output file.

Arrays are used instead of vectors because they are more efficient and the size of the data is
known ahead of time (at runtime).

## File I/O

File I/O is used to read the input file and write the output file. This lets users make small
edits to files without having to input every parameter again by hand, and the output is easier to
read and graph in Excel or a similar application.

## Design Decisions (Differences from the Proposal)

- The StandardFunctions library wasn't used because it's mostly for user inputs, and everything
  in this project is done at the command line.
- The number of years was not added to the `PanelData` struct — it's more realistic to have the
  number of years be the same for all panels, and it made constructing the 2D array easier.
- The `Equation` class differs from the proposal: it only has static member functions used by the
  simulation class. Each panel only uses the equation class once, while the simulation class uses
  it multiple times, so constructing objects per panel wasn't warranted. Standard test conditions
  are built into the code at compile time (`constexpr`) because there are so few cases where
  someone uses a different set of conditions that it didn't make sense to have them as variables.
- The `IOProcessor` class has more functions than in the proposal because of helper functions
  that weren't anticipated.

## Future Plans

- A graphing feature would be nice, but was beyond the scope of this project.
- Letting users input a location to pull irradiance and temperature data automatically would
  require internet access, which wasn't implemented here.

## Reflections

C++ proved harder than expected. There were issues with deallocating pointers multiple times (in
`main.cpp` and in a class destructor) that caused a fault, and getting the CSV formatting right
took the longest time. Takeaways: file input and output can simultaneously make projects easier
and more difficult; command-line arguments are useful (written before they were covered in
class); and separating classes you might want to extend later is beneficial. In the future, more
thought will go into planning — the UML diagram looks very different from the final code because
the right home for some functionality wasn't clear up front.

## Resources

- PVWatts and decay formulae from a friend's dissertation:
  https://repository.arizona.edu/handle/10150/677631
- cplusplus.com for input stream information used to process the input CSV
