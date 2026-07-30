#ifndef PANEL_MODELER_NET_PANELDATABASE_H
#define PANEL_MODELER_NET_PANELDATABASE_H

#include <QObject>
#include <QString>
#include <QUrl>

#include <optional>
#include <vector>

namespace panel_modeler {

    // Manufacturer specifications for one solar panel model, from the panel database.
    struct PanelSpec {
        QString manufacturer;
        QString model;
        double referencePower = 0.0;
        double tempDeratingCoeffPwr = 0.0;
        double decayRate = 0.0;

        [[nodiscard]] QString displayName() const { return manufacturer + QStringLiteral(" ") + model; }
    };

    // Loads solar panel specifications from a JSON database. A default database
    // ships with the application (see data/panels.json); users can also self-host
    // a database and point the GUI at its URL.
    //
    // Search order for the local database:
    //   1. $PANEL_MODELER_PANEL_DB (full path to a JSON file)
    //   2. panels.json next to the executable (running from the build tree)
    //   3. <install datadir>/panel-modeler/panels.json (installed copy)
    class PanelDatabase : public QObject {
        Q_OBJECT
    public:
        explicit PanelDatabase(QObject *parent = nullptr);

        // Loads the local database (see search order above).
        static std::vector<PanelSpec> loadLocal(QString *errorMessage = nullptr);

        // Path of the local database file that loadLocal() would use, for display.
        static QString localFilePath();

        // Asynchronously fetches a remote (self-hosted) database over HTTP(S);
        // emits loaded() or failed().
        void fetchRemote(const QUrl &url);

        // Parses a database document; exposed for reuse and testing.
        static std::optional<std::vector<PanelSpec>> parseJson(const QByteArray &payload, QString *errorMessage);

    signals:
        void loaded(const std::vector<panel_modeler::PanelSpec> &panels, const QString &source);
        void failed(const QString &message);
    };

} // namespace panel_modeler

#endif // PANEL_MODELER_NET_PANELDATABASE_H
