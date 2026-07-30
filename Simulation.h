#ifndef PANEL_MODELER_SIMULATION_H
#define PANEL_MODELER_SIMULATION_H

#include "PanelData.h"

class Simulation {
public:
  /**
   * @brief combines PVWatts equation with decay to find expected output after a
   * number of years
   * @param input input panel data (individual object, not array)
   * @param YEAR
   * @return expected output power after numYears years
   */
  static double decayPVWatts(const PanelData &input, unsigned long YEAR);
};

#endif // PANEL_MODELER_SIMULATION_H
