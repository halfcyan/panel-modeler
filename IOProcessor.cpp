#include "IOProcessor.h"
#include <sstream>
#include <iomanip>
#include <string>

std::string IOProcessor::trim(const std::string &s) {
    // see signature for documentation
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    if (start == s.size()) return std::string();
    size_t end = s.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) --end;
    return s.substr(start, end - start + 1);
}

double IOProcessor::parseDoubleSafe(const std::string &tok, const char *fieldName) {
    // see signature for documentation
    if (tok.empty()) {
        std::cerr << "Warning: missing value for " << fieldName << ". Using 0.0.\n";
        return 0.0;
    }
    try {
        return std::stod(tok);
    } catch ([[maybe_unused]] const std::exception &e) {
        std::cerr << "Warning: could not parse '" << tok << "' for " << fieldName << ". Using 0.0.\n";
        return 0.0;
    }
}

IOProcessor::IOProcessor(std::istream &inputStream) {
    _numLines = 0;
    _numYears = 0;

    // read first line as the number of years
    std::string firstLine;
    if (std::getline(inputStream, firstLine)) {
        const std::string TRIMMED = trim(firstLine);
        try {
            // making sure the number of years is sane
            if (const unsigned long PARSED = std::stoul(TRIMMED); PARSED > MAX_YEARS_ALLOWED) {
                std::cerr << "Warning: requested years (" << PARSED << ") exceeds maximum allowed (" << MAX_YEARS_ALLOWED
                    << "). Clamping to " << MAX_YEARS_ALLOWED << ".\n";
                _numYears = MAX_YEARS_ALLOWED;
            } else {
                _numYears = PARSED;
            }
        } catch ([[maybe_unused]] const std::exception &e) {
            // if it's not valid we're gonna throw an error but run with it at 0 years
            std::cerr << "Warning: could not parse number of years from first line ('" << firstLine
                << "'). Defaulting to 0.\n";
            _numYears = 0;
        }
    }

    // count the remaining lines for panels
    std::string line;
    // used cplusplus.com for this to grab the functions to count lines.
    while (std::getline(inputStream, line)) {
        if (std::string trimmed = trim(line); !trimmed.empty()) {
            _numLines++;
        }
    }

    inputStream.clear();
    inputStream.seekg(0);

    // skip the first line again
    std::getline(inputStream, firstLine);

    _panelArray = new PanelData[_numLines];
    CSVExtractor(inputStream);
}

IOProcessor::IOProcessor(const IOProcessor &other) {
    _numLines = other.getNumLines();
    _numYears = other.getNumYears();
    if (_numLines == 0) {
        _panelArray = nullptr;
    } else {
        _panelArray = new PanelData[_numLines];
        for (unsigned long int i = 0; i < _numLines; i++) {
            _panelArray[i] = other._panelArray[i];
        }
    }
}

IOProcessor &IOProcessor::operator=(const IOProcessor &other) {
    if (this == &other) return *this;

    // free existing
    delete[] _panelArray;

    _numLines = other.getNumLines();
    _numYears = other.getNumYears();

    if (_numLines == 0) {
        _panelArray = nullptr;
    } else {
        _panelArray = new PanelData[_numLines];
        for (unsigned long int i = 0; i < _numLines; i++) {
            _panelArray[i] = other._panelArray[i];
        }
    }

    return *this;
}

IOProcessor::~IOProcessor() { delete[] _panelArray; }

void IOProcessor::CSVExtractor(std::istream &inputStream) const {
    for (unsigned long int i = 0; i < _numLines; i++) {
        std::string line;
        if (!std::getline(inputStream, line)) {
            // fewer lines than expected; stop early
            break;
        }
        _panelArray[i] = PanelData{};
        if (std::string trimmed = trim(line); !trimmed.empty()) {
            _panelArray[i] = findPanelData(trimmed);
        }
    }
}

void IOProcessor::CSVFormatter(std::ostream &outputStream, const double **panelResults) const {
    const unsigned long MAX_YEARS = getMaxNumYears();
    const unsigned long NUM_PANELS = getNumLines();

    // for the header, the first column is the year label, then one column per panel
    outputStream << std::setw(10) << "Year";
    if (NUM_PANELS > 0) outputStream << ",";
    for (unsigned long p = 0; p < NUM_PANELS; p++) {
        // panel labels start at 1 for user-friendliness
        const std::string PANEL_LABEL = "Panel " + std::to_string(p + 1);
        // making sure the spacing lines everything up in the text output
        outputStream << std::setw(10) << PANEL_LABEL;
        if (p < NUM_PANELS - 1) outputStream << ",";
    }
    outputStream << std::endl;

    // one row per year, columns for each panel's value at that year
    for (unsigned long year = 0; year <= MAX_YEARS; year++) {
        const std::string YEAR_LABEL = "Year " + std::to_string(year);
        outputStream << std::setw(10) << YEAR_LABEL;
        if (NUM_PANELS > 0) outputStream << ",";

        for (unsigned long p = 0; p < NUM_PANELS; p++) {
            // making sure the spacing lines everything up in the text output
            outputStream << std::fixed << std::setprecision(4) << std::setw(10) << panelResults[p][year];
            if (p < NUM_PANELS - 1) outputStream << ",";
        }
        outputStream << std::endl;
    }
}

unsigned long int IOProcessor::getNumLines() const {
    return _numLines;
}

PanelData IOProcessor::findPanelData(const std::string &inputLine) {
    PanelData panelData{};
    std::stringstream ss(inputLine);
    std::string token;

    // used cplusplus.com here to find the stod and getline functions.
    // parse CSV tokens one by one and defensively convert to double.
    // if a token is missing or cannot be parsed, set the field to 0.0 and warn.
    if (std::getline(ss, token, ',')) panelData.referencePower = parseDoubleSafe(trim(token), "referencePower");
    else panelData.referencePower = 0.0;
    if (std::getline(ss, token, ',')) panelData.averageIrradiance = parseDoubleSafe(trim(token), "averageIrradiance");
    else panelData.averageIrradiance = 0.0;
    if (std::getline(ss, token, ',')) panelData.averageTemp = parseDoubleSafe(trim(token), "averageTemp");
    else panelData.averageTemp = 0.0;
    if (std::getline(ss, token, ',')) panelData.tempDeratingCoeffPwr = parseDoubleSafe(trim(token), "tempDeratingCoeffPwr");
    else panelData.tempDeratingCoeffPwr = 0.0;
    if (std::getline(ss, token, ',')) panelData.decayRate = parseDoubleSafe(trim(token), "decayRate");
    else panelData.decayRate = 0.0;

    // validation and clamping for all of the fields
    // values to clamp to are defined in the header file at compile
    if (panelData.referencePower < 0.0) {
        std::cerr << "Warning: referencePower < 0. Clamping to 0.0.\n";
        panelData.referencePower = 0.0;
    }

    if (panelData.averageIrradiance < 0.0) {
        std::cerr << "Warning: averageIrradiance < 0. Clamping to 0.0.\n";
        panelData.averageIrradiance = 0.0;
    } else if (panelData.averageIrradiance > MAX_IRRADIANCE) {
        std::cerr << "Warning: averageIrradiance unusually high. Clamping to " << MAX_IRRADIANCE << ".\n";
        panelData.averageIrradiance = MAX_IRRADIANCE;
    }

    if (panelData.averageTemp < MIN_TEMP) {
        std::cerr << "Warning: averageTemp unusually low. Clamping to " << MIN_TEMP << ".\n";
        panelData.averageTemp = MIN_TEMP;
    } else if (panelData.averageTemp > MAX_TEMP) {
        std::cerr << "Warning: averageTemp unusually high. Clamping to " << MAX_TEMP << ".\n";
        panelData.averageTemp = MAX_TEMP;
    }

    if (panelData.tempDeratingCoeffPwr < MIN_DERATING_COEFF) {
        std::cerr << "Warning: tempDeratingCoeffPwr < " << MIN_DERATING_COEFF << ". Clamping to " << MIN_DERATING_COEFF << ".\n";
        panelData.tempDeratingCoeffPwr = MIN_DERATING_COEFF;
    } else if (panelData.tempDeratingCoeffPwr > MAX_DERATING_COEFF) {
        std::cerr << "Warning: tempDeratingCoeffPwr > " << MAX_DERATING_COEFF << ". Clamping to " << MAX_DERATING_COEFF << ".\n";
        panelData.tempDeratingCoeffPwr = MAX_DERATING_COEFF;
    }

    if (panelData.decayRate < 0.0) {
        std::cerr << "Warning: decayRate < 0. Clamping to 0.0.\n";
        panelData.decayRate = 0.0;
    } else if (panelData.decayRate > MAX_DECAY_RATE) {
        std::cerr << "Warning: decayRate unusually high. Clamping to " << MAX_DECAY_RATE << ".\n";
        panelData.decayRate = MAX_DECAY_RATE;
    }

    return panelData;
}

PanelData *IOProcessor::getPanelArray() const {
    return _panelArray;
}

PanelData IOProcessor::getPanelArray(const size_t I) const {
    return _panelArray[I];
}

unsigned long int IOProcessor::getMaxNumYears() const {
    return _numYears;
}

unsigned long int IOProcessor::getNumYears() const {
    return _numYears;
}
