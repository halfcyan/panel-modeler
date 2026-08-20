#include "CsvIO.h"

#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace panel_modeler {

    void CsvIO::report(std::vector<std::string> *warnings, const std::string &message) {
        if (warnings != nullptr) {
            warnings->push_back(message);
        } else {
            std::cerr << "Warning: " << message << '\n';
        }
    }

    std::string CsvIO::trim(const std::string &s) {
        std::size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])) != 0) {
            ++start;
        }
        if (start == s.size()) {
            return {};
        }
        std::size_t end = s.size() - 1;
        while (end > start && std::isspace(static_cast<unsigned char>(s[end])) != 0) {
            --end;
        }
        return s.substr(start, end - start + 1);
    }

    double CsvIO::parseDoubleSafe(const std::string &token, const char *fieldName, std::vector<std::string> *warnings) {
        if (token.empty()) {
            report(warnings, std::string("missing value for ") + fieldName + ". Using 0.0.");
            return 0.0;
        }
        try {
            return std::stod(token);
        } catch (const std::exception &) {
            report(warnings, "could not parse '" + token + "' for " + fieldName + ". Using 0.0.");
            return 0.0;
        }
    }

    SimulationInput CsvIO::readInput(std::istream &input, std::vector<std::string> *warnings) {
        SimulationInput result;

        // first line is the number of years to simulate (same for all panels)
        std::string line;
        if (std::getline(input, line)) {
            const std::string trimmed = trim(line);
            try {
                const unsigned long parsed = std::stoul(trimmed);
                if (parsed > MAX_YEARS_ALLOWED) {
                    report(warnings,
                           "requested years (" + std::to_string(parsed) + ") exceeds maximum allowed (" +
                               std::to_string(MAX_YEARS_ALLOWED) + "). Clamping to " +
                               std::to_string(MAX_YEARS_ALLOWED) + ".");
                    result.years = MAX_YEARS_ALLOWED;
                } else {
                    result.years = parsed;
                }
            } catch (const std::exception &) {
                report(warnings, "could not parse number of years from first line ('" + line + "'). Defaulting to 0.");
                result.years = 0;
            }
        }

        // every remaining non-empty line describes one panel
        while (std::getline(input, line)) {
            if (const std::string trimmed = trim(line); !trimmed.empty()) {
                result.panels.push_back(parsePanelLine(trimmed, warnings));
            }
        }
        return result;
    }

    PanelData CsvIO::parsePanelLine(const std::string &line, std::vector<std::string> *warnings) {
        PanelData panel{};
        std::stringstream stream(line);
        std::string token;

        // parse CSV tokens one by one; the optional sixth token is the array count.
        // Numeric values accept arbitrary decimal precision;
        // a missing or unparseable token becomes 0.0 with a warning
        if (std::getline(stream, token, ',')) {
            panel.referencePower = parseDoubleSafe(trim(token), "referencePower", warnings);
        } else {
            panel.referencePower = 0.0;
        }
        if (std::getline(stream, token, ',')) {
            panel.averageIrradiance = parseDoubleSafe(trim(token), "averageIrradiance", warnings);
        } else {
            panel.averageIrradiance = 0.0;
        }
        if (std::getline(stream, token, ',')) {
            panel.averageTemp = parseDoubleSafe(trim(token), "averageTemp", warnings);
        } else {
            panel.averageTemp = 0.0;
        }
        if (std::getline(stream, token, ',')) {
            panel.tempDeratingCoeffPwr = parseDoubleSafe(trim(token), "tempDeratingCoeffPwr", warnings);
        } else {
            panel.tempDeratingCoeffPwr = 0.0;
        }
        if (std::getline(stream, token, ',')) {
            panel.decayRate = parseDoubleSafe(trim(token), "decayRate", warnings);
        } else {
            panel.decayRate = 0.0;
        }
        if (std::getline(stream, token, ',')) {
            const std::string countToken = trim(token);
            try {
                std::size_t consumed = 0;
                const unsigned long parsed = std::stoul(countToken, &consumed);
                if (consumed != countToken.size()) {
                    throw std::invalid_argument("panelCount must be an integer");
                }
                if (parsed == 0UL) {
                    report(warnings, "panelCount must be at least 1. Using 1.");
                } else if (parsed > MAX_PANEL_COUNT) {
                    report(warnings,
                           "panelCount exceeds maximum allowed. Clamping to " + std::to_string(MAX_PANEL_COUNT) + ".");
                    panel.panelCount = MAX_PANEL_COUNT;
                } else {
                    panel.panelCount = static_cast<unsigned int>(parsed);
                }
            } catch (const std::exception &) {
                report(warnings, "could not parse '" + countToken + "' for panelCount. Using 1.");
            }
        }

        // clamp every field to the limits in the header so absurd inputs cannot
        // produce silently absurd outputs
        if (panel.referencePower < 0.0) {
            report(warnings, "referencePower < 0. Clamping to 0.0.");
            panel.referencePower = 0.0;
        }

        if (panel.averageIrradiance < 0.0) {
            report(warnings, "averageIrradiance < 0. Clamping to 0.0.");
            panel.averageIrradiance = 0.0;
        } else if (panel.averageIrradiance > MAX_IRRADIANCE) {
            report(warnings, "averageIrradiance unusually high. Clamping to " + std::to_string(MAX_IRRADIANCE) + ".");
            panel.averageIrradiance = MAX_IRRADIANCE;
        }

        if (panel.averageTemp < MIN_TEMP) {
            report(warnings, "averageTemp unusually low. Clamping to " + std::to_string(MIN_TEMP) + ".");
            panel.averageTemp = MIN_TEMP;
        } else if (panel.averageTemp > MAX_TEMP) {
            report(warnings, "averageTemp unusually high. Clamping to " + std::to_string(MAX_TEMP) + ".");
            panel.averageTemp = MAX_TEMP;
        }

        if (panel.tempDeratingCoeffPwr < MIN_DERATING_COEFF) {
            report(warnings,
                   "tempDeratingCoeffPwr < " + std::to_string(MIN_DERATING_COEFF) + ". Clamping to " +
                       std::to_string(MIN_DERATING_COEFF) + ".");
            panel.tempDeratingCoeffPwr = MIN_DERATING_COEFF;
        } else if (panel.tempDeratingCoeffPwr > MAX_DERATING_COEFF) {
            report(warnings,
                   "tempDeratingCoeffPwr > " + std::to_string(MAX_DERATING_COEFF) + ". Clamping to " +
                       std::to_string(MAX_DERATING_COEFF) + ".");
            panel.tempDeratingCoeffPwr = MAX_DERATING_COEFF;
        }

        if (panel.decayRate < 0.0) {
            report(warnings, "decayRate < 0. Clamping to 0.0.");
            panel.decayRate = 0.0;
        } else if (panel.decayRate > MAX_DECAY_RATE) {
            report(warnings, "decayRate unusually high. Clamping to " + std::to_string(MAX_DECAY_RATE) + ".");
            panel.decayRate = MAX_DECAY_RATE;
        }

        return panel;
    }

    void CsvIO::writeResults(std::ostream &output, const std::vector<std::vector<double>> &results) {
        const std::size_t numPanels = results.size();
        const std::size_t maxYears = numPanels > 0 ? results.front().size() : 0;

        // header: first column is the year label, then one column per panel
        output << std::setw(10) << "Year";
        if (numPanels > 0) {
            output << ",";
        }
        for (std::size_t panel = 0; panel < numPanels; ++panel) {
            // panel labels start at 1 for user-friendliness
            output << std::setw(10) << "Panel " + std::to_string(panel + 1);
            if (panel < numPanels - 1) {
                output << ",";
            }
        }
        output << "\n";

        // one row per year, one column per panel value at that year
        for (std::size_t year = 0; year < maxYears; ++year) {
            output << std::setw(10) << "Year " + std::to_string(year);
            if (numPanels > 0) {
                output << ",";
            }
            for (std::size_t panel = 0; panel < numPanels; ++panel) {
                output << std::fixed << std::setprecision(4) << std::setw(10) << results[panel][year];
                if (panel < numPanels - 1) {
                    output << ",";
                }
            }
            output << "\n";
        }
    }

} // namespace panel_modeler
