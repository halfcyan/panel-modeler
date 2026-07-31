#include "MainWindow.h"

#include <QDoubleSpinBox>
#include <QFileDialog>

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

#include <fstream>

#include "config.h"
#include "core/CsvIO.h"
#include "core/Simulation.h"
#include "gui/ChartWidget.h"
#include "gui/PanelDatabaseDialog.h"
#include "net/ClimateService.h"
#include "net/Geocoder.h"
#include "net/PanelDatabase.h"

namespace panel_modeler {

    namespace {
        constexpr int COLUMN_NAME = 0;
        constexpr int COLUMN_POWER = 1;
        constexpr int COLUMN_DERATE = 2;
        constexpr int COLUMN_DECAY = 3;
        constexpr int NUM_PANEL_COLUMNS = 4;
    } // namespace

    MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), m_addressEdit(new QLineEdit(this)), m_latitudeSpin(new QDoubleSpinBox(this)),
        m_longitudeSpin(new QDoubleSpinBox(this)), m_irradianceSpin(new QDoubleSpinBox(this)),
        m_temperatureSpin(new QDoubleSpinBox(this)),
        m_geocodeButton(new QPushButton(QStringLiteral("Geocode && Fetch"), this)),
        m_fetchClimateButton(new QPushButton(QStringLiteral("Fetch Climate"), this)),
        m_statusLabel(
            new QLabel(QStringLiteral("Enter an address or coordinates, or edit the values manually."), this)),
        m_panelTable(new QTableWidget(0, NUM_PANEL_COLUMNS, this)), m_yearsSpin(new QSpinBox(this)),
        m_exportButton(new QPushButton(QStringLiteral("Export Results CSV..."), this)),
        m_resultsTabs(new QTabWidget(this)), m_resultsTable(new QTableWidget(0, 0, this)),
        m_chart(new ChartWidget(this)), m_geocoder(new Geocoder(this)), m_climateService(new ClimateService(this)) {
        setWindowTitle(QStringLiteral("panel-modeler ") + QStringLiteral(PANEL_MODELER_VERSION));
        resize(900, 720);

        // ---- location group ----
        m_addressEdit->setPlaceholderText(QStringLiteral("City, address, or place name (e.g. Phoenix, AZ)"));
        m_statusLabel->setWordWrap(true);
        m_statusLabel->setTextFormat(Qt::PlainText);
        m_latitudeSpin->setRange(-90.0, 90.0);
        m_latitudeSpin->setDecimals(6);
        m_latitudeSpin->setSingleStep(0.1);
        m_longitudeSpin->setRange(-180.0, 180.0);
        m_longitudeSpin->setDecimals(6);
        m_longitudeSpin->setSingleStep(0.1);
        m_irradianceSpin->setRange(0.0, CsvIO::MAX_IRRADIANCE);
        m_irradianceSpin->setDecimals(2);
        m_irradianceSpin->setSuffix(QStringLiteral(" W/m^2"));
        m_temperatureSpin->setRange(CsvIO::MIN_TEMP, CsvIO::MAX_TEMP);
        m_temperatureSpin->setDecimals(2);
        m_temperatureSpin->setSuffix(QStringLiteral(" °C"));
        m_latitudeSpin->setMinimumWidth(130);
        m_longitudeSpin->setMinimumWidth(130);
        m_irradianceSpin->setMinimumWidth(150);
        m_temperatureSpin->setMinimumWidth(130);

        auto *locationGrid = new QGridLayout;
        locationGrid->setHorizontalSpacing(12);
        locationGrid->setVerticalSpacing(8);
        locationGrid->setColumnStretch(1, 1);
        locationGrid->setColumnStretch(3, 1);

        locationGrid->addWidget(new QLabel(QStringLiteral("Address or place:")), 0, 0);
        locationGrid->addWidget(m_addressEdit, 0, 1, 1, 3);
        locationGrid->addWidget(m_geocodeButton, 0, 4);

        locationGrid->addWidget(new QLabel(QStringLiteral("Latitude:")), 1, 0);
        locationGrid->addWidget(m_latitudeSpin, 1, 1);
        locationGrid->addWidget(new QLabel(QStringLiteral("Longitude:")), 1, 2);
        locationGrid->addWidget(m_longitudeSpin, 1, 3);
        locationGrid->addWidget(m_fetchClimateButton, 1, 4);

        locationGrid->addWidget(new QLabel(QStringLiteral("Annual irradiance:")), 2, 0);
        locationGrid->addWidget(m_irradianceSpin, 2, 1);
        locationGrid->addWidget(new QLabel(QStringLiteral("Annual temperature:")), 2, 2);
        locationGrid->addWidget(m_temperatureSpin, 2, 3);

        locationGrid->addWidget(m_statusLabel, 3, 0, 1, 5);

        auto *locationGroup = new QGroupBox(QStringLiteral("Site Location & Climate"), this);
        locationGroup->setLayout(locationGrid);

        // ---- panels group ----
        m_panelTable->setHorizontalHeaderLabels({QStringLiteral("Name"), QStringLiteral("Reference Power (W)"),
                                                 QStringLiteral("Temp Derate (1/°C)"), QStringLiteral("Decay (1/yr)")});
        m_panelTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_panelTable->setSelectionBehavior(QAbstractItemView::SelectRows);

        auto *addButton = new QPushButton(QStringLiteral("Add Panel"), this);
        auto *removeButton = new QPushButton(QStringLiteral("Remove Selected"), this);
        auto *databaseButton = new QPushButton(QStringLiteral("Load from Database..."), this);
        auto *importButton = new QPushButton(QStringLiteral("Import CSV..."), this);
        auto *panelButtons = new QHBoxLayout;
        panelButtons->addWidget(addButton);
        panelButtons->addWidget(removeButton);
        panelButtons->addStretch(1);
        panelButtons->addWidget(databaseButton);
        panelButtons->addWidget(importButton);

        auto *panelLayout = new QVBoxLayout;
        panelLayout->addWidget(m_panelTable);
        panelLayout->addLayout(panelButtons);
        auto *panelsGroup = new QGroupBox(QStringLiteral("Panels"), this);
        panelsGroup->setLayout(panelLayout);

        // ---- simulation row ----
        m_yearsSpin->setRange(0, static_cast<int>(CsvIO::MAX_YEARS_ALLOWED));
        m_yearsSpin->setValue(25);
        auto *runButton = new QPushButton(QStringLiteral("Run Simulation"), this);
        m_exportButton->setEnabled(false);
        auto *simulationRow = new QHBoxLayout;
        simulationRow->addWidget(new QLabel(QStringLiteral("Years to simulate:")));
        simulationRow->addWidget(m_yearsSpin);
        simulationRow->addWidget(runButton);
        simulationRow->addStretch(1);
        simulationRow->addWidget(m_exportButton);

        // ---- results ----
        m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_resultsTabs->addTab(m_resultsTable, QStringLiteral("Table"));
        m_resultsTabs->addTab(m_chart, QStringLiteral("Chart"));

        auto *centralLayout = new QVBoxLayout;
        centralLayout->addWidget(locationGroup);
        centralLayout->addWidget(panelsGroup, 1);
        centralLayout->addLayout(simulationRow);
        centralLayout->addWidget(m_resultsTabs, 1);
        auto *central = new QWidget(this);
        central->setLayout(centralLayout);
        setCentralWidget(central);

        // ---- menus ----
        QMenu *fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
        QAction *importAction = fileMenu->addAction(QStringLiteral("&Import CSV..."), this, [this]() { importCsv(); });
        importAction->setShortcut(QKeySequence::Open);
        QAction *exportAction =
            fileMenu->addAction(QStringLiteral("&Export Results CSV..."), this, [this]() { exportResults(); });
        exportAction->setShortcut(QKeySequence::Save);
        fileMenu->addSeparator();
        fileMenu->addAction(QStringLiteral("&Quit"), QKeySequence::Quit, this, &QWidget::close);

        QMenu *toolsMenu = menuBar()->addMenu(QStringLiteral("&Tools"));
        toolsMenu->addAction(QStringLiteral("Panel &Database..."), this, [this]() { openPanelDatabase(); });

        QMenu *helpMenu = menuBar()->addMenu(QStringLiteral("&Help"));
        helpMenu->addAction(QStringLiteral("&About"), this, [this]() {
            QMessageBox::about(
                this, QStringLiteral("About panel-modeler"),
                QStringLiteral("panel-modeler %1\n\nPredicts solar panel power output over time using the "
                               "PVWatts equation plus exponential decay.\n\nClimate data: NASA POWER climatology "
                               "(2001-2020).\nAddress geocoding: OpenStreetMap Nominatim.\nPanel database values "
                               "are nominal; verify against manufacturer datasheets.")
                    .arg(QStringLiteral(PANEL_MODELER_VERSION)));
        });

        // ---- behavior ----
        connect(m_geocodeButton, &QPushButton::clicked, this, [this]() { geocodeAddress(); });
        connect(m_addressEdit, &QLineEdit::returnPressed, this, [this]() { geocodeAddress(); });
        connect(m_fetchClimateButton, &QPushButton::clicked, this, [this]() { fetchClimate(); });
        connect(addButton, &QPushButton::clicked, this, [this]() { addPanel(); });
        connect(removeButton, &QPushButton::clicked, this, [this]() { removeSelectedPanels(); });
        connect(databaseButton, &QPushButton::clicked, this, [this]() { openPanelDatabase(); });
        connect(runButton, &QPushButton::clicked, this, [this]() { runSimulation(); });
        connect(m_exportButton, &QPushButton::clicked, this, [this]() { exportResults(); });

        connect(m_geocoder, &Geocoder::found, this, [this](const GeoCoordinate &coordinate) {
            m_latitudeSpin->setValue(coordinate.latitude);
            m_longitudeSpin->setValue(coordinate.longitude);
            setStatus(QStringLiteral("Geocoded: %1 — fetching climate...").arg(coordinate.displayName));
            fetchClimate();
        });
        connect(m_geocoder, &Geocoder::failed, this, [this](const QString &message) {
            setBusy(false);
            setStatus(QStringLiteral("Geocoding failed: %1").arg(message));
        });
        connect(m_climateService, &ClimateService::fetched, this, [this](const ClimateData &climate) {
            setBusy(false);
            m_irradianceSpin->setValue(climate.irradianceWm2);
            m_temperatureSpin->setValue(climate.temperatureC);
            setStatus(
                QStringLiteral("Climate data fetched from NASA POWER (2001-2020 averages). Values are editable."));
        });
        connect(m_climateService, &ClimateService::failed, this, [this](const QString &message) {
            setBusy(false);
            setStatus(QStringLiteral("Climate lookup failed: %1").arg(message));
        });

        // a sensible starting row so the app is usable immediately
        appendPanelRow(QStringLiteral("Panel 1"), 400.0, -0.0035, 0.005);
    }

    void MainWindow::setBusy(const bool busy) {
        m_geocodeButton->setEnabled(!busy);
        m_fetchClimateButton->setEnabled(!busy);
    }

    void MainWindow::setStatus(const QString &message) { m_statusLabel->setText(message); }

    void MainWindow::geocodeAddress() {
        const QString address = m_addressEdit->text().trimmed();
        if (address.isEmpty()) {
            setStatus(QStringLiteral("Enter an address first (or enter coordinates and use Fetch Climate)."));
            return;
        }
        setBusy(true);
        setStatus(QStringLiteral("Geocoding \"%1\"...").arg(address));
        m_geocoder->geocodeAddress(address);
    }

    void MainWindow::fetchClimate() {
        setBusy(true);
        setStatus(QStringLiteral("Fetching climate data from NASA POWER..."));
        m_climateService->fetchClimate(m_latitudeSpin->value(), m_longitudeSpin->value());
    }

    void MainWindow::addPanel() {
        appendPanelRow(QStringLiteral("Panel %1").arg(m_panelTable->rowCount() + 1), 400.0, -0.0035, 0.005);
    }

    void MainWindow::appendPanelRow(const QString &name, const double referencePower, const double deratingCoeff,
                                    const double decayRate) {
        const int row = m_panelTable->rowCount();
        m_panelTable->insertRow(row);
        m_panelTable->setItem(row, COLUMN_NAME, new QTableWidgetItem(name));
        auto *powerItem = new QTableWidgetItem;
        powerItem->setData(Qt::EditRole, referencePower);
        m_panelTable->setItem(row, COLUMN_POWER, powerItem);
        auto *derateItem = new QTableWidgetItem;
        derateItem->setData(Qt::EditRole, deratingCoeff);
        m_panelTable->setItem(row, COLUMN_DERATE, derateItem);
        auto *decayItem = new QTableWidgetItem;
        decayItem->setData(Qt::EditRole, decayRate);
        m_panelTable->setItem(row, COLUMN_DECAY, decayItem);
    }

    void MainWindow::removeSelectedPanels() {
        const QModelIndexList selection = m_panelTable->selectionModel()->selectedRows();
        if (selection.isEmpty()) {
            setStatus(QStringLiteral("Select one or more panel rows to remove."));
            return;
        }
        // remove from bottom to top so row numbers stay valid
        QList<int> rows;
        for (const QModelIndex &index: selection) {
            rows.append(index.row());
        }
        std::sort(rows.begin(), rows.end(), std::greater<>());
        for (const int row: rows) {
            m_panelTable->removeRow(row);
        }
    }

    void MainWindow::openPanelDatabase() {
        PanelDatabaseDialog dialog(this);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }
        // keep the optional alive for the whole scope rather than binding a temporary
        const std::optional<PanelSpec> selected = dialog.selectedPanel();
        if (!selected.has_value()) {
            return;
        }
        appendPanelRow(selected->displayName(), selected->referencePower, selected->tempDeratingCoeffPwr,
                       selected->decayRate);
        setStatus(QStringLiteral("Added %1 from the panel database.").arg(selected->displayName()));
    }

    void MainWindow::importCsv() {
        const QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("Import input CSV"), QString(),
                                                              QStringLiteral("CSV files (*.csv);;All files (*)"));
        if (fileName.isEmpty()) {
            return;
        }
        std::ifstream inputFile(fileName.toStdString());
        if (!inputFile.is_open()) {
            QMessageBox::warning(this, QStringLiteral("Import CSV"),
                                 QStringLiteral("Could not open %1.").arg(fileName));
            return;
        }

        std::vector<std::string> warnings;
        const SimulationInput input = CsvIO::readInput(inputFile, &warnings);
        if (input.panels.empty()) {
            QMessageBox::warning(this, QStringLiteral("Import CSV"),
                                 QStringLiteral("No panel data found in that file."));
            return;
        }

        m_panelTable->setRowCount(0);
        QStringList names;
        for (std::size_t index = 0; index < input.panels.size(); ++index) {
            const PanelData &panel = input.panels[index];
            appendPanelRow(QStringLiteral("Panel %1").arg(index + 1), panel.referencePower, panel.tempDeratingCoeffPwr,
                           panel.decayRate);
        }
        m_yearsSpin->setValue(static_cast<int>(input.years));

        // irradiance/temperature are per-location in the GUI; take them from the
        // first row and mention it if the file disagrees between rows
        m_irradianceSpin->setValue(input.panels.front().averageIrradiance);
        m_temperatureSpin->setValue(input.panels.front().averageTemp);
        for (const PanelData &panel: input.panels) {
            if (panel.averageIrradiance != input.panels.front().averageIrradiance ||
                panel.averageTemp != input.panels.front().averageTemp) {
                warnings.emplace_back("rows in this file use different irradiance/temperature values; "
                                      "the GUI applies one location to all panels (first row used)");
                break;
            }
        }

        if (!warnings.empty()) {
            QStringList lines;
            for (const std::string &warning: warnings) {
                lines.append(QString::fromStdString(warning));
            }
            QMessageBox::information(this, QStringLiteral("Import CSV — warnings"), lines.join(QStringLiteral("\n")));
        }
        setStatus(QStringLiteral("Imported %1 panels from %2.").arg(input.panels.size()).arg(fileName));
    }

    std::optional<std::vector<PanelData>> MainWindow::collectPanels(QString *error) {
        std::vector<PanelData> panels;
        for (int row = 0; row < m_panelTable->rowCount(); ++row) {
            const QTableWidgetItem *nameItem = m_panelTable->item(row, COLUMN_NAME);
            bool powerOk = false;
            bool derateOk = false;
            bool decayOk = false;
            PanelData panel{};
            panel.averageIrradiance = m_irradianceSpin->value();
            panel.averageTemp = m_temperatureSpin->value();
            panel.referencePower = m_panelTable->item(row, COLUMN_POWER)->text().toDouble(&powerOk);
            panel.tempDeratingCoeffPwr = m_panelTable->item(row, COLUMN_DERATE)->text().toDouble(&derateOk);
            panel.decayRate = m_panelTable->item(row, COLUMN_DECAY)->text().toDouble(&decayOk);
            if (!powerOk || !derateOk || !decayOk) {
                const QString name = nameItem != nullptr ? nameItem->text() : QString::number(row + 1);
                *error = QStringLiteral("Row %1 (\"%2\") has a non-numeric value.").arg(row + 1).arg(name);
                m_panelTable->selectRow(row);
                return std::nullopt;
            }
            panels.push_back(panel);
        }
        if (panels.empty()) {
            *error = QStringLiteral("Add at least one panel first.");
            return std::nullopt;
        }
        return panels;
    }

    void MainWindow::runSimulation() {
        QString error;
        const auto panels = collectPanels(&error);
        if (!panels.has_value()) {
            setStatus(error);
            return;
        }

        const unsigned long years = static_cast<unsigned long>(m_yearsSpin->value());
        m_results = Simulation::run(*panels, years);

        QStringList names;
        for (int row = 0; row < m_panelTable->rowCount(); ++row) {
            const QTableWidgetItem *item = m_panelTable->item(row, COLUMN_NAME);
            names.append(item != nullptr && !item->text().isEmpty() ? item->text()
                                                                    : QStringLiteral("Panel %1").arg(row + 1));
        }
        showResults(m_results, names, years);
        setStatus(QStringLiteral("Simulated %1 panels over %2 years.").arg(panels->size()).arg(years));
    }

    void MainWindow::showResults(const std::vector<std::vector<double>> &results, const QStringList &names,
                                 const unsigned long years) {
        m_resultsTable->setRowCount(0);
        m_resultsTable->setColumnCount(0);
        m_resultsTable->setColumnCount(static_cast<int>(results.size()) + 1);

        QStringList headers{QStringLiteral("Year")};
        headers.append(names);
        m_resultsTable->setHorizontalHeaderLabels(headers);

        for (unsigned long year = 0; year <= years; ++year) {
            const int row = m_resultsTable->rowCount();
            m_resultsTable->insertRow(row);
            m_resultsTable->setItem(row, 0, new QTableWidgetItem(QString::number(year)));
            for (std::size_t panel = 0; panel < results.size(); ++panel) {
                m_resultsTable->setItem(row, static_cast<int>(panel) + 1,
                                        new QTableWidgetItem(QString::number(results[panel][year], 'f', 4)));
            }
        }
        m_resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        m_resultsTable->resizeColumnsToContents();

        m_chart->setData(results, names);
        m_exportButton->setEnabled(true);
        m_resultsTabs->setCurrentWidget(m_resultsTable);
    }

    void MainWindow::exportResults() {
        if (m_results.empty()) {
            return;
        }
        const QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("Export results CSV"), QString(),
                                                              QStringLiteral("CSV files (*.csv);;All files (*)"));
        if (fileName.isEmpty()) {
            return;
        }
        std::ofstream outputFile(fileName.toStdString());
        if (!outputFile.is_open()) {
            QMessageBox::warning(this, QStringLiteral("Export results"),
                                 QStringLiteral("Could not open %1 for writing.").arg(fileName));
            return;
        }
        CsvIO::writeResults(outputFile, m_results);
        setStatus(QStringLiteral("Results written to %1.").arg(fileName));
    }

} // namespace panel_modeler
