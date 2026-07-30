#include "Geocoder.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

#include "config.h"

namespace panel_modeler {

    namespace {

        QNetworkRequest nominatimRequest(const QUrl &url) {
            QNetworkRequest request(url);
            // Nominatim's usage policy requires an identifying User-Agent
            request.setHeader(QNetworkRequest::UserAgentHeader,
                              QStringLiteral("panel-modeler/") + QStringLiteral(PANEL_MODELER_VERSION));
            request.setTransferTimeout(15000);
            return request;
        }

    } // namespace

    Geocoder::Geocoder(QObject *parent) : QObject(parent) {}

    QUrl Geocoder::buildUrl(const QString &address) {
        QUrl url(QStringLiteral("https://nominatim.openstreetmap.org/search"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("q"), address);
        query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
        query.addQueryItem(QStringLiteral("limit"), QStringLiteral("1"));
        url.setQuery(query);
        return url;
    }

    void Geocoder::geocodeAddress(const QString &address) {
        auto *manager = new QNetworkAccessManager(this);
        QNetworkReply *reply = manager->get(nominatimRequest(buildUrl(address)));
        connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
            reply->deleteLater();
            manager->deleteLater();
            QString error;
            const auto coordinate = parseReply(reply->readAll(), &error);
            if (coordinate.has_value()) {
                emit found(*coordinate);
            } else {
                emit failed(error);
            }
        });
    }

    std::optional<GeoCoordinate> Geocoder::geocodeAddressBlocking(const QString &address, QString *errorMessage) {
        QNetworkAccessManager manager;
        QNetworkReply *reply = manager.get(nominatimRequest(buildUrl(address)));

        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        const QByteArray payload = reply->readAll();
        reply->deleteLater();
        return parseReply(payload, errorMessage);
    }

    std::optional<GeoCoordinate> Geocoder::parseReply(const QByteArray &payload, QString *errorMessage) {
        QJsonParseError parseError{};
        const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("could not parse geocoding response: %1").arg(parseError.errorString());
            }
            return std::nullopt;
        }
        const QJsonArray results = document.array();
        if (results.isEmpty()) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("no results for that address");
            }
            return std::nullopt;
        }

        const QJsonObject first = results.first().toObject();
        bool latOk = false;
        bool lonOk = false;
        GeoCoordinate coordinate;
        coordinate.latitude = first.value(QStringLiteral("lat")).toString().toDouble(&latOk);
        coordinate.longitude = first.value(QStringLiteral("lon")).toString().toDouble(&lonOk);
        coordinate.displayName = first.value(QStringLiteral("display_name")).toString();
        if (!latOk || !lonOk) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("geocoding response had invalid coordinates");
            }
            return std::nullopt;
        }
        return coordinate;
    }

} // namespace panel_modeler
