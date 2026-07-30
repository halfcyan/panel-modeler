#ifndef PANEL_MODELER_FILEIO_H
#define PANEL_MODELER_FILEIO_H

#include <iostream>
#include <string>

class FileIO {
public:
  static std::istream *fileInput(const std::string &INPUT_FILE_NAME);

  static std::ostream *fileOutput(const std::string &OUTPUT_FILE_NAME);
};

#endif // PANEL_MODELER_FILEIO_H
