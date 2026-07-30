#include "PanelDatabase.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <cstdlib>

#include "config.h"

namespace panel_modeler {

    PanelDatabase::PanelDatabase(QObject *parent) : QObject(parent) {}

    QString PanelDatabase::localFilePath() {
        if (const QByteArray override = qgetenv("PANEL_MODELER_PANEL_DB"); !override.isEmpty()) {
            return QString::fromLocal8Bit(override);
        }
        const QString besideExecutable = QCoreApplication::applicationDirPath() + QStringLiteral("/panels.json");
        if (QFile::exists(besideExecutable)) {
            return besideExecutable;
        }
        return QStringLiteral(PANEL_MODELER_DATA_DIR) + QStringLiteral("/panels.json");
    }

    std::vector<PanelSpec> PanelDatabase::loadLocal(QString *errorMessage) {
        const QString path = localFilePath();
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("could not open panel database %1: %2").arg(path, file.errorString());
            }
            return {};
        }
        const auto panels = parseJson(file.readAll(), errorMessage);
        return panels.value_or(std::vector<PanelSpec>{});
    }

    void PanelDatabase::fetchRemote(const QUrl &url) {
        auto *manager = new QNetworkAccessManager(this);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader,
                          QStringLiteral("panel-modeler/") + QStringLiteral(PANEL_MODELER_VERSION));
        request.setTransferTimeout(15000);

        QNetworkReply *reply = manager->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply, manager, url]() {
            reply->deleteLater();
            manager->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit failed(QStringLiteral("could not fetch %1: %2").arg(url.toString(), reply->errorString()));
                return;
            }
            QString error;
            const auto panels = parseJson(reply->readAll(), &error);
            if (panels.has_value()) {
                emit loaded(*panels, url.toString());
            } else {
                emit failed(error);
            }
        });
    }

    std::optional<std::vector<PanelSpec>> PanelDatabase::parseJson(const QByteArray &payload, QString *errorMessage) {
        QJsonParseError parseError{};
        const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("could not parse panel database: %1").arg(parseError.errorString());
            }
            return std::nullopt;
        }

        const QJsonArray entries = document.object().value(QStringLiteral("panels")).toArray();
        if (entries.isEmpty()) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("panel database has no 'panels' entries");
            }
            return std::nullopt;
        }

        std::vector<PanelSpec> panels;
        panels.reserve(static_cast<std::size_t>(entries.size()));
        for (const QJsonValue &entry: entries) {
            const QJsonObject object = entry.toObject();
            PanelSpec spec;
            spec.manufacturer = object.value(QStringLiteral("manufacturer")).toString();
            spec.model = object.value(QStringLiteral("model")).toString();
            spec.referencePower = object.value(QStringLiteral("referencePower")).toDouble(0.0);
            spec.tempDeratingCoeffPwr = object.value(QStringLiteral("tempDeratingCoeffPwr")).toDouble(0.0);
            spec.decayRate = object.value(QStringLiteral("decayRate")).toDouble(0.0);
            if (spec.manufacturer.isEmpty() || spec.model.isEmpty() || spec.referencePower <= 0.0) {
                if (errorMessage != nullptr) {
                    *errorMessage = QStringLiteral("panel database entry is missing a name or has invalid power");
                }
                return std::nullopt;
            }
            panels.push_back(spec);
        }
        return panels;
    }

} // namespace panel_modeler
