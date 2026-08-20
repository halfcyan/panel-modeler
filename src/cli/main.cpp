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
 *
 * Since version 2.0 the program can also fetch irradiance and temperature for
 * you: pass --climate with either "latitude,longitude" or an address and it
 * prints the annual averages from NASA POWER, ready to paste into an input CSV.
 */

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "config.h"
#include "core/CsvIO.h"
#include "core/Simulation.h"

#ifdef PANEL_MODELER_WITH_NETWORK
#include <QCoreApplication>
#include <QString>

#include "net/ClimateService.h"
#include "net/Geocoder.h"
#endif

namespace {

    void printUsage() {
        std::cout << "Usage: panel-modeler inputFile outputFile\n"
                     "  inputFile: path to the input file in csv format.\n"
                     "  outputFile: path to the output file.\n"
#ifdef PANEL_MODELER_WITH_NETWORK
                     "\n"
                     "       panel-modeler --climate <latitude,longitude | \"address\">\n"
                     "  Looks up annual average irradiance and temperature for a location\n"
                     "  (NASA POWER; addresses are geocoded with OpenStreetMap Nominatim) and\n"
                     "  prints a ready-to-edit input csv line.\n"
#endif
                     "\n"
                     "       panel-modeler --version\n"
                     "\n"
                     "Format the input file with the first line as the number of years for all panels,\n"
                     "followed by one line per panel type or array.\n"
                     "List the parameters in the following order:\n"
                     "1. Reference power per panel as a decimal\n"
                     "2. Average irradiance in watts per square meter as a decimal\n"
                     "3. Average temperature in degrees Celsius as a decimal\n"
                     "4. Temperature Derating Coefficient of Power as a decimal\n"
                     "5. Decay rate per year of your panel as a decimal\n"
                     "6. Optional number of identical panels in the array (defaults to 1)\n"
                     "\n"
                     "All of these parameters should be available from your solar panel manufacturer!\n"
                     "\n"
                     "The output will be a csv file with columns for each panel and a row for each year.\n";
    }

    int runSimulation(const std::string &inputFileName, const std::string &outputFileName) {
        std::ifstream inputFile(inputFileName);
        if (!inputFile.is_open()) {
            std::cerr << "Error opening file " << inputFileName << '\n';
            return 1;
        }
        std::ofstream outputFile(outputFileName);
        if (!outputFile.is_open()) {
            std::cerr << "Error opening file " << outputFileName << '\n';
            return 1;
        }

        const panel_modeler::SimulationInput input = panel_modeler::CsvIO::readInput(inputFile);
        if (input.panels.empty()) {
            std::cerr << "No panel data found in input file; no output generated.\n";
            return 0;
        }

        const std::vector<std::vector<double>> results = panel_modeler::Simulation::run(input.panels, input.years);
        panel_modeler::CsvIO::writeResults(outputFile, results);
        return 0;
    }

#ifdef PANEL_MODELER_WITH_NETWORK
    // Parses "lat,lon" from the given spec; returns false if it is not a coordinate pair.
    bool parseCoordinates(const std::string &spec, double &latitude, double &longitude) {
        std::stringstream stream(spec);
        std::string latToken;
        std::string lonToken;
        if (!std::getline(stream, latToken, ',') || !std::getline(stream, lonToken)) {
            return false;
        }
        try {
            latitude = std::stod(latToken);
            longitude = std::stod(lonToken);
        } catch (const std::exception &) {
            return false;
        }
        return latitude >= -90.0 && latitude <= 90.0 && longitude >= -180.0 && longitude <= 180.0;
    }

    int runClimateLookup(const std::string &spec) {
        double latitude = 0.0;
        double longitude = 0.0;
        QString displayName;

        if (parseCoordinates(spec, latitude, longitude)) {
            displayName = QStringLiteral("%1, %2").arg(latitude).arg(longitude);
        } else {
            QString error;
            const auto coordinate =
                panel_modeler::Geocoder::geocodeAddressBlocking(QString::fromStdString(spec), &error);
            if (!coordinate.has_value()) {
                std::cerr << "Geocoding failed: " << error.toStdString() << '\n';
                return 1;
            }
            latitude = coordinate->latitude;
            longitude = coordinate->longitude;
            displayName = coordinate->displayName;
        }

        QString error;
        const auto climate = panel_modeler::ClimateService::fetchClimateBlocking(latitude, longitude, &error);
        if (!climate.has_value()) {
            std::cerr << "Climate lookup failed: " << error.toStdString() << '\n';
            return 1;
        }

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Location: " << displayName.toStdString() << '\n';
        std::cout << "Annual average irradiance: " << climate->irradianceWm2 << " W/m^2 ("
                  << climate->irradianceKwhM2Day << " kWh/m^2/day)\n";
        std::cout << "Annual average temperature: " << climate->temperatureC << " deg C\n";
        std::cout << "Source: NASA POWER monthly climatology (2001-2020)\n\n";
        std::cout << "Ready-to-edit input csv line (set power, derating and decay to match your panel):\n";
        std::cout << "400.0, " << climate->irradianceWm2 << ", " << climate->temperatureC << ", -0.0035, 0.005\n";
        return 0;
    }
#endif // PANEL_MODELER_WITH_NETWORK

} // namespace

int main(int argc, char *argv[]) {
#ifdef PANEL_MODELER_WITH_NETWORK
    // QCoreApplication is only needed for the event loop used by --climate;
    // plain CSV simulation never touches Qt.
    QCoreApplication app(argc, argv);
#endif

#ifdef PANEL_MODELER_WITH_NETWORK
    // checked before generic 2-argument CSV mode because --climate also takes one value
    if (argc == 3 && std::string(argv[1]) == "--climate") {
        return runClimateLookup(argv[2]);
    }
#endif

    if (argc == 3) {
        return runSimulation(argv[1], argv[2]);
    }

    if (argc == 2) {
        const std::string argument = argv[1];
        if (argument == "--version") {
            std::cout << "panel-modeler " << PANEL_MODELER_VERSION << '\n';
            return 0;
        }
#ifdef PANEL_MODELER_WITH_NETWORK
        if (argument == "--climate") {
            std::cerr << "--climate needs a value: <latitude,longitude> or an address.\n";
            return 1;
        }
#endif
        if (argument != "help" && argument != "--help" && argument != "-h") {
            std::cerr << "Unrecognized argument: " << argument << "\n\n";
        }
        printUsage();
        return 0;
    }

    if (argc != 1) {
        std::cerr << "You have entered the incorrect number of parameters. Please try again.\n\n";
    }
    printUsage();
    return 0;
}
