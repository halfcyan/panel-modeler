#include "Simulation.h"

#include "Equation.h"

namespace panel_modeler {

    double Simulation::decayPVWatts(const PanelData &input, const unsigned long year) {
        return Equation::PVWatts(input) * Equation::powerDecayPrediction(input, year);
    }

    std::vector<std::vector<double>> Simulation::run(const std::vector<PanelData> &panels, const unsigned long years) {
        std::vector<std::vector<double>> results(panels.size());
        for (std::size_t panel = 0; panel < panels.size(); ++panel) {
            results[panel].reserve(years + 1);
            for (unsigned long year = 0; year <= years; ++year) {
                results[panel].push_back(decayPVWatts(panels[panel], year));
            }
        }
        return results;
    }

} // namespace panel_modeler
