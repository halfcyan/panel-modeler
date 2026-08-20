#ifndef PANEL_MODELER_CORE_PANELDATA_H
#define PANEL_MODELER_CORE_PANELDATA_H

namespace panel_modeler {

    // Specifications for a single solar panel plus the climate it operates in.
    // All values are plain decimals, matching the CSV input format.
    struct PanelData {
        double referencePower; // rated power at standard test conditions (W)
        double averageIrradiance; // annual average irradiance (W/m^2)
        double averageTemp; // annual average temperature (°C)
        double tempDeratingCoeffPwr; // temperature derating coefficient of power (1/°C, typically negative)
        double decayRate; // fractional output loss per year (1/yr)
        unsigned int panelCount = 1; // number of identical panels in the array
    };

} // namespace panel_modeler

#endif // PANEL_MODELER_CORE_PANELDATA_H
