#ifndef PANEL_MODELER_IOPROCESSOR_H
#define PANEL_MODELER_IOPROCESSOR_H

#include "PanelData.h"
#include <iostream>

class IOProcessor {
public:
  /**
   * @brief constructs the IOProcessor object based on an input stream and runs
   * the CSV extractor on that stream
   * @param inputStream input stream to construct the IOProcessor based on
   */
  explicit IOProcessor(std::istream &inputStream);

  /**
   * @brief copy constructor
   * @param other object to be copied
   */
  IOProcessor(const IOProcessor &other);

  /**
   * @brief copy assignment operator
   */
  IOProcessor &operator=(const IOProcessor &other);

  /**
   * @brief destructor
   */
  ~IOProcessor();

  /**
   * @brief extracts panel data from csv, first line by line then constructing
   * an array
   * @param inputStream input stream to extract data from
   */
  void CSVExtractor(std::istream &inputStream) const;

  /**
   * @brief formats input data from panelResults to csv to export to
   * outputStream
   * @param outputStream output stream to push data to
   * @param panelResults 2D array of results from simulation
   * (panelResults[panel][year])
   */
  void CSVFormatter(std::ostream &outputStream,
                    const double **panelResults) const;

  /**
   * @brief extracts panel's given parameters from a comma-deliminated line
   * @param inputLine line from CSV to pull information from
   * @return single PanelData object with data from CSV line
   */
  static PanelData findPanelData(const std::string &inputLine);

  /**
   * @brief getter for number of lines in the input
   * @return number of lines
   */
  [[nodiscard]] unsigned long int getNumLines() const;

  /**
   * @brief getter for the PanelData array
   * @return array of PanelData objects for each line in CSV
   */
  [[nodiscard]] PanelData *getPanelArray() const;

  /**
   * @brief getter for PanelData array at an index
   *
   * @param I index to return
   * @return array of PanelData objects for each line in CSV
   */
  [[nodiscard]] PanelData getPanelArray(size_t I) const;

  /**
   * @brief getter for the maximum number of years across all panels
   * @return maximum numYears value from all panels
   */
  [[nodiscard]] unsigned long int getMaxNumYears() const;

  /**
   * @brief getter for the number of years (read from first line of input)
   * @return numYears value from input file
   */
  [[nodiscard]] unsigned long int getNumYears() const;

private:
  /**
   * @brief helper function to trim whitespace from both ends of a string
   * @param s input string to trim
   * @return trimmed copy of the string
   */
  static std::string trim(const std::string &s);

  /**
   * @brief helper function to convert string to double with warnings on failure
   * @param tok token to convert
   * @param fieldName name of field for diagnostic messages
   * @return parsed double or 0.0 on failure
   */
  static double parseDoubleSafe(const std::string &tok, const char *fieldName);

  // validation limits for input parameters
  // i prefer having them all in one spot so i can change them easily
  static constexpr unsigned long MAX_YEARS_ALLOWED = 100UL;
  static constexpr double MAX_IRRADIANCE = 2000.0;
  static constexpr double MIN_TEMP = -50.0;
  static constexpr double MAX_TEMP = 60.0;
  static constexpr double MAX_DERATING_COEFF = 0.05;
  static constexpr double MIN_DERATING_COEFF = -0.05;
  static constexpr double MAX_DECAY_RATE = 0.10;
  unsigned long int _numLines; // number of lines in array
  unsigned long int _numYears; // number of years (read from first line)
  PanelData *_panelArray; // dynamically allocated array of PanelData objects
};

#endif // PANEL_MODELER_IOPROCESSOR_H
