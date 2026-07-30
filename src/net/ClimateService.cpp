#include "ClimateService.h"

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

        QNetworkRequest powerRequest(const QUrl &url) {
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::UserAgentHeader,
                              QStringLiteral("panel-modeler/") + QStringLiteral(PANEL_MODELER_VERSION));
            request.setTransferTimeout(15000);
            return request;
        }

        // NASA POWER reports errors as a JSON body with a "messages" array, even on
        // non-200 responses; surface the first message when present.
        QString extractServerMessage(const QByteArray &payload) {
            const QJsonDocument document = QJsonDocument::fromJson(payload);
            const QJsonArray messages = document.object().value(QStringLiteral("messages")).toArray();
            if (!messages.isEmpty()) {
                return messages.first().toString();
            }
            return {};
        }

    } // namespace

    ClimateService::ClimateService(QObject *parent) : QObject(parent) {}

    QUrl ClimateService::buildUrl(const double latitude, const double longitude) {
        QUrl url(QStringLiteral("https://power.larc.nasa.gov/api/temporal/climatology/point"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("parameters"), QStringLiteral("ALLSKY_SFC_SW_DWN,T2M"));
        query.addQueryItem(QStringLiteral("community"), QStringLiteral("RE"));
        query.addQueryItem(QStringLiteral("longitude"), QString::number(longitude));
        query.addQueryItem(QStringLiteral("latitude"), QString::number(latitude));
        query.addQueryItem(QStringLiteral("format"), QStringLiteral("JSON"));
        url.setQuery(query);
        return url;
    }

    void ClimateService::fetchClimate(const double latitude, const double longitude) {
        auto *manager = new QNetworkAccessManager(this);
        QNetworkReply *reply = manager->get(powerRequest(buildUrl(latitude, longitude)));
        connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
            reply->deleteLater();
            manager->deleteLater();

            QString error;
            const QByteArray payload = reply->readAll();
            if (reply->error() != QNetworkReply::NoError) {
                const QString serverMessage = extractServerMessage(payload);
                emit failed(serverMessage.isEmpty() ? reply->errorString() : serverMessage);
                return;
            }
            const auto climate = parseReply(payload, &error);
            if (climate.has_value()) {
                emit fetched(*climate);
            } else {
                emit failed(error);
            }
        });
    }

    std::optional<ClimateData> ClimateService::fetchClimateBlocking(const double latitude, const double longitude,
                                                                    QString *errorMessage) {
        QNetworkAccessManager manager;
        QNetworkReply *reply = manager.get(powerRequest(buildUrl(latitude, longitude)));

        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        const QByteArray payload = reply->readAll();
        const QString transportError = reply->error() != QNetworkReply::NoError ? reply->errorString() : QString();
        reply->deleteLater();

        if (!transportError.isEmpty()) {
            if (errorMessage != nullptr) {
                const QString serverMessage = extractServerMessage(payload);
                *errorMessage = serverMessage.isEmpty() ? transportError : serverMessage;
            }
            return std::nullopt;
        }
        return parseReply(payload, errorMessage);
    }

    std::optional<ClimateData> ClimateService::parseReply(const QByteArray &payload, QString *errorMessage) {
        QJsonParseError parseError{};
        const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("could not parse NASA POWER response: %1").arg(parseError.errorString());
            }
            return std::nullopt;
        }

        const QJsonObject parameters = document.object()
                                           .value(QStringLiteral("properties"))
                                           .toObject()
                                           .value(QStringLiteral("parameter"))
                                           .toObject();
        // -999 is NASA POWER's documented fill value for missing data
        constexpr double FILL_VALUE = -999.0;
        const double irradiance = parameters.value(QStringLiteral("ALLSKY_SFC_SW_DWN"))
                                      .toObject()
                                      .value(QStringLiteral("ANN"))
                                      .toDouble(-1.0);
        const double temperature =
            parameters.value(QStringLiteral("T2M")).toObject().value(QStringLiteral("ANN")).toDouble(FILL_VALUE);

        if (irradiance < 0.0 || temperature <= FILL_VALUE + 1.0) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("NASA POWER has no data for that location");
            }
            return std::nullopt;
        }

        ClimateData climate;
        climate.irradianceKwhM2Day = irradiance;
        climate.irradianceWm2 = irradiance * KWH_M2_DAY_TO_W_M2;
        climate.temperatureC = temperature;
        return climate;
    }

} // namespace panel_modeler
