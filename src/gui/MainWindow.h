#ifndef PANEL_MODELER_GUI_MAINWINDOW_H
#define PANEL_MODELER_GUI_MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>

#include <optional>
#include <vector>

#include "core/PanelData.h"

class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTabWidget;

namespace panel_modeler {

    struct PanelSpec;
    class ChartWidget;
    class ClimateService;
    class Geocoder;

    class MainWindow : public QMainWindow {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget *parent = nullptr);

    private:
        // location + network
        void geocodeAddress();
        void fetchClimate();
        void setBusy(bool busy);
        void setStatus(const QString &message);

        // panel table
        void addPanel();
        void removeSelectedPanels();
        void openPanelDatabase();
        void importCsv();
        void appendPanelRow(const QString &name, double referencePower, double deratingCoeff, double decayRate);

        // simulation
        void runSimulation();
        void exportResults();
        // reads the panel table; sets *error and returns nullopt on the first invalid cell
        std::optional<std::vector<PanelData>> collectPanels(QString *error);
        void showResults(const std::vector<std::vector<double>> &results, const QStringList &names,
                         unsigned long years);

        QLineEdit *m_addressEdit;
        QDoubleSpinBox *m_latitudeSpin;
        QDoubleSpinBox *m_longitudeSpin;
        QDoubleSpinBox *m_irradianceSpin;
        QDoubleSpinBox *m_temperatureSpin;
        QPushButton *m_geocodeButton;
        QPushButton *m_fetchClimateButton;
        QLabel *m_statusLabel;

        QTableWidget *m_panelTable;
        QSpinBox *m_yearsSpin;
        QPushButton *m_exportButton;

        QTabWidget *m_resultsTabs;
        QTableWidget *m_resultsTable;
        ChartWidget *m_chart;

        Geocoder *m_geocoder;
        ClimateService *m_climateService;

        // last simulation run, kept for CSV export; indexed by [panel][year]
        std::vector<std::vector<double>> m_results;
    };

} // namespace panel_modeler

#endif // PANEL_MODELER_GUI_MAINWINDOW_H
