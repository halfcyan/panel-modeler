#include "Equation.h"

#include <cmath>

// I am deeply grateful to Kyle Boyer for helping me figure out what equations I could use for this.
// I knew what I needed to do, but I didn't know what equations to use for that.
// He pointed me in the right direction.

namespace panel_modeler {

    double Equation::PVWatts(const PanelData &input) {
        return (input.referencePower * (input.averageIrradiance / STD_IRRADIANCE)) *
            (1 + (input.tempDeratingCoeffPwr * (input.averageTemp - STD_TEMP)));
    }

    double Equation::powerDecayPrediction(const PanelData &input, const unsigned long years) {
        return std::pow(1 - input.decayRate, static_cast<double>(years));
    }

} // namespace panel_modeler
