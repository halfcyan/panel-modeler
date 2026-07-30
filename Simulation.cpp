#include "Simulation.h"
#include "Equation.h"
#include <cmath>

double Simulation::decayPVWatts(const PanelData &input, const unsigned long YEAR) {
    return Equation::PVWatts(input) * pow(1 - input.decayRate, YEAR);
}
