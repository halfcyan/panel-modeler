#ifndef PANEL_MODELER_NET_CLIMATESERVICE_H
#define PANEL_MODELER_NET_CLIMATESERVICE_H

#include <QObject>
#include <QString>

#include <optional>

namespace panel_modeler {

    // Annual average climate figures for a location, as returned by NASA POWER.
    struct ClimateData {
        double irradianceWm2 = 0.0; // annual average irradiance converted to W/m^2
        double irradianceKwhM2Day = 0.0; // annual average irradiance as reported (kWh/m^2/day)
        double temperatureC = 0.0; // annual average temperature at 2 m (°C)
    };

    // Fetches 20-year monthly/annual climatologies (solar irradiance and 2 m air
    // temperature) from the NASA POWER API (https://power.larc.nasa.gov).
    // Free, no API key required.
    class ClimateService : public QObject {
        Q_OBJECT
    public:
        explicit ClimateService(QObject *parent = nullptr);

        // Asynchronously fetches climate data; emits fetched() or failed().
        void fetchClimate(double latitude, double longitude);

        // Synchronous variant for the CLI; blocks until the reply finishes.
        static std::optional<ClimateData> fetchClimateBlocking(double latitude, double longitude,
                                                               QString *errorMessage = nullptr);

    signals:
        void fetched(const panel_modeler::ClimateData &climate);
        void failed(const QString &message);

    private:
        // NASA POWER reports irradiance as kWh/m^2/day; the simulation wants W/m^2
        static constexpr double KWH_M2_DAY_TO_W_M2 = 1000.0 / 24.0;

        static QUrl buildUrl(double latitude, double longitude);
        static std::optional<ClimateData> parseReply(const QByteArray &payload, QString *errorMessage);
    };

} // namespace panel_modeler

#endif // PANEL_MODELER_NET_CLIMATESERVICE_H
