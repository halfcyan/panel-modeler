#include "Equation.h"
#include <cmath>


// I am deeply grateful to Kyle Boyer for helping me figure out what equations I could use for this.
// I knew what I needed to do, but I didn't know what equations to use for that.
// He pointed me in the right direction.

double Equation::PVWatts(const PanelData &input) {
    return input.referencePower * (input.averageIrradiance / STD_IIR) *
        (1 + input.tempDeratingCoeffPwr * (input.averageTemp - STD_TEMP));
}

double Equation::powerDecayPrediction(const PanelData &input, const unsigned long YEARS) { return pow(1 - input.decayRate, YEARS); }
