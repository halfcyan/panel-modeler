#ifndef PANEL_MODELER_NET_GEOCODER_H
#define PANEL_MODELER_NET_GEOCODER_H

#include <QObject>
#include <QString>

#include <optional>

namespace panel_modeler {

    // A resolved location: coordinates plus a human-readable name.
    struct GeoCoordinate {
        double latitude = 0.0;
        double longitude = 0.0;
        QString displayName;
    };

    // Turns a postal address into coordinates using OpenStreetMap's Nominatim
    // service (https://nominatim.openstreetmap.org). Free, no API key; please see
    // the usage policy in the README before hammering it.
    class Geocoder : public QObject {
        Q_OBJECT
    public:
        explicit Geocoder(QObject *parent = nullptr);

        // Asynchronously geocodes an address; emits found() or failed().
        void geocodeAddress(const QString &address);

        // Synchronous variant for the CLI; blocks until the reply finishes.
        static std::optional<GeoCoordinate> geocodeAddressBlocking(const QString &address,
                                                                   QString *errorMessage = nullptr);

    signals:
        void found(const panel_modeler::GeoCoordinate &coordinate);
        void failed(const QString &message);

    private:
        static QUrl buildUrl(const QString &address);
        static std::optional<GeoCoordinate> parseReply(const QByteArray &payload, QString *errorMessage);
    };

} // namespace panel_modeler

#endif // PANEL_MODELER_NET_GEOCODER_H
