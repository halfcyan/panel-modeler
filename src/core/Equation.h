#ifndef PANEL_MODELER_CORE_EQUATION_H
#define PANEL_MODELER_CORE_EQUATION_H

#include "PanelData.h"

namespace panel_modeler {

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
         * @brief calculates the fraction of power output remaining after a number of
         * years relative to a panel's decay rate
         * @param input reference panel data to run the prediction on
         * @param years number of years to predict decay for
         * @return coefficient, maximum 1, of maximum power after a certain number of
         * years
         */
        [[nodiscard]] static double powerDecayPrediction(const PanelData &input, unsigned long years);

    private:
        // reference conditions used by PVWatts; fixed at compile time because
        // practically all panel datasheets use these standard test conditions
        static constexpr double STD_TEMP = 25.0; // °C
        static constexpr double STD_IRRADIANCE = 1000.0; // W/m^2
    };

} // namespace panel_modeler

#endif // PANEL_MODELER_CORE_EQUATION_H
