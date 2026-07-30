#ifndef PANEL_MODELER_EQUATION_H
#define PANEL_MODELER_EQUATION_H

#include "PanelData.h"

class Equation {
public:
  /**
   * @brief runs the PVWatts equation on a given solar panel to find expected
   * output in a situation
   * @param input reference panel data to run PVWatts on
   * @return expected output power
   */
  [[nodiscard]] static double PVWatts(const PanelData &input);

  /**
   * @brief calculates the amount of power output remaining after the number of
   * years relative to a panel's decay rate
   * @param input reference panel data to run the prediction on
   * @param YEARS number of years to predict decay for
   * @return coefficient, maximum 1, of maximum power after a certain number of
   * years
   */
  [[nodiscard]] static double powerDecayPrediction(const PanelData &input,
                                                   unsigned long YEARS);

private:
  // these are used in PVWatts as the reference temperature and irradiance
  // constexpr because they won't change beyond compile time
  static constexpr unsigned long STD_TEMP = 25;
  static constexpr unsigned long STD_IIR = 1000;
};

#endif // PANEL_MODELER_EQUATION_H
