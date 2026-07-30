#ifndef PANEL_MODELER_CORE_CSVIO_H
#define PANEL_MODELER_CORE_CSVIO_H

#include "PanelData.h"

#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace panel_modeler {

    // Everything the CLI/GUI needs to run a simulation, as read from an input CSV:
    // the number of years (first line) plus one panel per following line.
    struct SimulationInput {
        unsigned long years = 0;
        std::vector<PanelData> panels;
    };

    class CsvIO {
    public:
        /**
         * @brief reads the number of years and the panel list from an input stream
         * @param input stream containing the input CSV
         * @param warnings if non-null, parse/clamp warnings are appended here;
         * if null, they are printed to std::cerr instead
         * @return parsed years + panels (invalid values are clamped, never fatal)
         */
        [[nodiscard]] static SimulationInput readInput(std::istream &input,
                                                       std::vector<std::string> *warnings = nullptr);

        /**
         * @brief parses one comma-delimited panel line into a PanelData
         * @param line line from an input CSV (order: power, irradiance, temperature,
         * derating coefficient, decay rate)
         * @param warnings see readInput
         * @return parsed and clamped panel data
         */
        [[nodiscard]] static PanelData parsePanelLine(const std::string &line,
                                                      std::vector<std::string> *warnings = nullptr);

        /**
         * @brief writes simulation results as a CSV: header row, one column per
         * panel, one row per year starting at Year 0. The format is kept stable so
         * existing spreadsheets/expected-output files keep matching.
         * @param output stream to write to
         * @param results simulation results indexed by [panel][year]
         */
        static void writeResults(std::ostream &output, const std::vector<std::vector<double>> &results);

        // validation limits for input parameters, kept in one place so they are easy to change
        static constexpr unsigned long MAX_YEARS_ALLOWED = 100UL;
        static constexpr double MAX_IRRADIANCE = 2000.0;
        static constexpr double MIN_TEMP = -50.0;
        static constexpr double MAX_TEMP = 60.0;
        static constexpr double MAX_DERATING_COEFF = 0.05;
        static constexpr double MIN_DERATING_COEFF = -0.05;
        static constexpr double MAX_DECAY_RATE = 0.10;

    private:
        static std::string trim(const std::string &s);
        static double parseDoubleSafe(const std::string &token, const char *fieldName,
                                      std::vector<std::string> *warnings);
        static void report(std::vector<std::string> *warnings, const std::string &message);
    };

} // namespace panel_modeler

#endif // PANEL_MODELER_CORE_CSVIO_H
