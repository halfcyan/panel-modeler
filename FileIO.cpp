#include "FileIO.h"
#include <fstream>

std::istream *FileIO::fileInput(const std::string &INPUT_FILE_NAME) {
    std::ifstream *inputFile = new std::ifstream(INPUT_FILE_NAME);
    if (!inputFile->is_open()) {
        std::cerr << "Error opening file " << INPUT_FILE_NAME << std::endl;
        return nullptr;
    }
    return inputFile;
}

std::ostream *FileIO::fileOutput(const std::string &OUTPUT_FILE_NAME) {
    std::ofstream *outputFile = new std::ofstream(OUTPUT_FILE_NAME);
    if (!outputFile->is_open()) {
        std::cerr << "Error opening file " << OUTPUT_FILE_NAME << std::endl;
        return nullptr;
    }
    return outputFile;
}
