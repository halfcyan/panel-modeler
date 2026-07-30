/* CSCI 200: Final
 *
 * Author: Cypress Reed
 *
 * Resources used (Office Hours, Tutoring, Other Students, etc. & in what
 * capacity): I used my friend's dissertation to grab the PVWatts and decay
 * formulae. Those are available online, but I knew his dissertation had that
 * information. Here's a link to that dissertation:
 * https://repository.arizona.edu/handle/10150/677631
 *
 *     I also used cplusplus.com for the input stream information so I could
 * process the input csv properly.
 *
 * This program takes an input set of panel data and outputs a simulation of
 * that panel's output after a given number of years at a specific location. It
 * uses the irradiance per square meter and average daytime temperature at a
 * location for this. If you'd like to gather that information yourself, you can
 * use the tool SAM from the National Renewable Energy Lab (I will not call it
 * NLR) It's a robust tool that does a much more complex version of what this
 * program does. It's also written in C++. I didn't reference its code, but I
 * did use it to find the real irradiance and temperature at the location of my
 * parents' house for their system for one of the reference points. If you're
 * curious, it's the third line. They use panels with that information! The
 * other lines are just values that are sensible and based on other panels I
 * could find commercially.
 */

#include "FileIO.h"
#include "IOProcessor.h"
#include "PanelData.h"
#include "Simulation.h"
#include <cstring>
#include <fstream>
#include <iostream>

int main(const int ARGC, char *argv[]) {
  if (ARGC != 3) {
    if (ARGC != 1) {
      if (strcmp(argv[1], "help") != 0) {
        std::cerr << "You have entered the incorrect number of parameters. "
                     "Please try again."
                  << std::endl;
      }
    }
    std::cout << "Usage: panel-modeler inputFile outputFile" << std::endl;
    std::cout << "inputFile: a string to a relative filepath of the input file "
                 "in csv format."
              << std::endl;
    std::cout
        << "outputFile: a string to a relative filepath of the output file."
        << std::endl
        << std::endl;
    std::cout << "Format the input file with the first line as the number of "
                 "years for all panels,"
              << std::endl;
    std::cout << "followed by lines for each target panel or situation"
              << std::endl;
    std::cout << "List the parameters in the following order:" << std::endl;
    std::cout << "1. Reference power as a decimal" << std::endl;
    std::cout << "2. Average irradiance in watts per square meter as a decimal"
              << std::endl;
    std::cout << "3. Average temperature in degrees Celsius as a decimal"
              << std::endl;
    std::cout << "4. Temperature Derating Coefficient of Power as a decimal"
              << std::endl;
    std::cout << "5. Decay rate per year of your panel as a decimal"
              << std::endl
              << std::endl;
    std::cout << "All of these parameters should be available from your solar "
                 "panel manufacturer!"
              << std::endl
              << std::endl;
    std::cout << "The output will be a csv file with columns for each panel "
                 "and a row for each year."
              << std::endl;
    return 0;
  }
  const std::string INPUT_FILE_NAME = argv[1];
  const std::string OUTPUT_FILE_NAME = argv[2];

  std::istream *inputFileStream = FileIO::fileInput(INPUT_FILE_NAME);
  if (inputFileStream == nullptr) {
    return 1;
  }
  std::ostream *outputFileStream = FileIO::fileOutput(OUTPUT_FILE_NAME);
  if (outputFileStream == nullptr) {
    // clean up previously opened input stream
    delete inputFileStream;
    return 1;
  }

  const IOProcessor PROCESSOR(*inputFileStream);
  const unsigned long MAX_YEARS = PROCESSOR.getMaxNumYears();

  // If there are no panels in the input, inform the user and exit cleanly
  if (PROCESSOR.getNumLines() == 0) {
    std::cerr << "No panel data found in input file; no output generated.\n";
    delete inputFileStream;
    delete outputFileStream;
    return 0;
  }

  // Allocate 2D array: panelPowerOutputArray[panel][year]
  double **panelPowerOutputArray = new double *[PROCESSOR.getNumLines()];
  for (size_t i = 0; i < PROCESSOR.getNumLines(); i++) {
    const PanelData PANEL_DATA = PROCESSOR.getPanelArray(i);
    // Allocate space for all years (same for all panels)
    panelPowerOutputArray[i] = new double[MAX_YEARS + 1];

    for (unsigned long year = 0; year <= MAX_YEARS; year++) {
      panelPowerOutputArray[i][year] =
          Simulation::decayPVWatts(PANEL_DATA, year);
    }
  }

  PROCESSOR.CSVFormatter(*outputFileStream,
                         const_cast<const double **>(panelPowerOutputArray));

  // deallocate properly
  for (size_t i = 0; i < PROCESSOR.getNumLines(); i++) {
    delete[] panelPowerOutputArray[i];
  }
  delete[] panelPowerOutputArray;
  // close and delete file streams allocated by FileIO
  delete inputFileStream;
  delete outputFileStream;
  return 0;
}
