#ifndef PANEL_MODELER_CORE_SIMULATION_H
#define PANEL_MODELER_CORE_SIMULATION_H

#include "PanelData.h"

#include <vector>

namespace panel_modeler {

    class Simulation {
    public:
        /**
         * @brief combines the PVWatts equation with decay to find expected output
         * after a number of years
         * @param input panel data, including the number of identical panels in the array
         * @param year number of years into the future
         * @return expected output power after `year` years
         */
        [[nodiscard]] static double decayPVWatts(const PanelData &input, unsigned long year);

        /**
         * @brief runs decayPVWatts for every panel and every year from 0 to `years`
         * @param panels panels to simulate
         * @param years number of years to simulate (same for all panels)
         * @return results indexed by [panel][year]
         */
        [[nodiscard]] static std::vector<std::vector<double>> run(const std::vector<PanelData> &panels,
                                                                  unsigned long years);
    };

} // namespace panel_modeler

#endif // PANEL_MODELER_CORE_SIMULATION_H
