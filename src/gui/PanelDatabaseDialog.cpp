#include "PanelDatabaseDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTableWidget>
#include <QVBoxLayout>

namespace panel_modeler {

    namespace {
        constexpr int SOURCE_LOCAL = 0;
        constexpr int SOURCE_REMOTE = 1;
    } // namespace

    PanelDatabaseDialog::PanelDatabaseDialog(QWidget *parent) :
        QDialog(parent), m_sourceCombo(new QComboBox(this)), m_urlEdit(new QLineEdit(this)),
        m_reloadButton(new QPushButton(QStringLiteral("Reload"), this)), m_filterEdit(new QLineEdit(this)),
        m_table(new QTableWidget(0, 5, this)), m_database(new PanelDatabase(this)) {
        setWindowTitle(QStringLiteral("Panel Database"));
        resize(640, 420);

        m_sourceCombo->addItem(QStringLiteral("Bundled / installed database"), SOURCE_LOCAL);
        m_sourceCombo->addItem(QStringLiteral("Remote (self-hosted) URL"), SOURCE_REMOTE);
        m_urlEdit->setPlaceholderText(QStringLiteral("https://your-server.example/panels.json"));
        m_urlEdit->setText(
            QSettings()
                .value(QStringLiteral("panelDb/remoteUrl"), QStringLiteral("https://panels.willowidk.dev/panels.json"))
                .toString());
        m_urlEdit->setEnabled(false);
        m_filterEdit->setPlaceholderText(QStringLiteral("Filter by manufacturer or model..."));

        m_table->setHorizontalHeaderLabels({QStringLiteral("Manufacturer"), QStringLiteral("Model"),
                                            QStringLiteral("Power (W)"), QStringLiteral("Derate (decimal %/°C)"),
                                            QStringLiteral("Decay (decimal %/yr)")});
        m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_table->setSelectionMode(QAbstractItemView::SingleSelection);
        m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_table->verticalHeader()->setVisible(false);

        auto *sourceLayout = new QFormLayout;
        sourceLayout->addRow(QStringLiteral("Source:"), m_sourceCombo);
        sourceLayout->addRow(QStringLiteral("URL:"), m_urlEdit);

        auto *topLayout = new QVBoxLayout;
        topLayout->addLayout(sourceLayout);
        auto *filterLayout = new QHBoxLayout;
        filterLayout->addWidget(m_filterEdit, 1);
        filterLayout->addWidget(m_reloadButton);
        topLayout->addLayout(filterLayout);
        topLayout->addWidget(m_table, 1);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        topLayout->addWidget(buttons);

        setLayout(topLayout);

        connect(m_sourceCombo, &QComboBox::currentIndexChanged, this, [this](const int index) {
            m_urlEdit->setEnabled(m_sourceCombo->itemData(index).toInt() == SOURCE_REMOTE);
            loadFromCurrentSource();
        });
        connect(m_reloadButton, &QPushButton::clicked, this, [this]() { loadFromCurrentSource(); });
        connect(m_filterEdit, &QLineEdit::textChanged, this, [this]() { applyFilter(); });
        connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int, int) { onAccept(); });
        connect(buttons, &QDialogButtonBox::accepted, this, [this]() { onAccept(); });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        connect(m_database, &PanelDatabase::loaded, this,
                [this](const std::vector<PanelSpec> &panels, const QString &source) {
                    QSettings().setValue(QStringLiteral("panelDb/remoteUrl"), m_urlEdit->text());
                    populate(panels, source);
                });
        connect(m_database, &PanelDatabase::failed, this, [this](const QString &message) {
            QMessageBox::warning(this, QStringLiteral("Panel Database"), message);
        });

        loadFromCurrentSource();
    }

    std::optional<PanelSpec> PanelDatabaseDialog::selectedPanel() const { return m_selected; }

    void PanelDatabaseDialog::loadFromCurrentSource() {
        if (m_sourceCombo->currentData().toInt() == SOURCE_REMOTE) {
            const QUrl url = QUrl::fromUserInput(m_urlEdit->text());
            if (!url.isValid() || url.isEmpty()) {
                QMessageBox::warning(this, QStringLiteral("Panel Database"),
                                     QStringLiteral("Enter a valid URL first."));
                return;
            }
            m_database->fetchRemote(url);
            return;
        }

        QString error;
        const std::vector<PanelSpec> panels = PanelDatabase::loadLocal(&error);
        if (panels.empty()) {
            QMessageBox::warning(this, QStringLiteral("Panel Database"), error);
            return;
        }
        populate(panels, PanelDatabase::localFilePath());
    }

    void PanelDatabaseDialog::populate(const std::vector<PanelSpec> &panels, const QString &source) {
        m_panels = panels;
        m_table->setRowCount(0);
        for (const PanelSpec &spec: m_panels) {
            const int row = m_table->rowCount();
            m_table->insertRow(row);
            m_table->setItem(row, 0, new QTableWidgetItem(spec.manufacturer));
            m_table->setItem(row, 1, new QTableWidgetItem(spec.model));
            m_table->setItem(row, 2, new QTableWidgetItem(QString::number(spec.referencePower)));
            m_table->setItem(row, 3, new QTableWidgetItem(QString::number(spec.tempDeratingCoeffPwr)));
            m_table->setItem(row, 4, new QTableWidgetItem(QString::number(spec.decayRate)));
        }
        setWindowTitle(QStringLiteral("Panel Database — %1").arg(source));
        applyFilter();
    }

    void PanelDatabaseDialog::applyFilter() {
        const QString filter = m_filterEdit->text();
        for (int row = 0; row < m_table->rowCount(); ++row) {
            const QString text = m_table->item(row, 0)->text() + QLatin1Char(' ') + m_table->item(row, 1)->text();
            m_table->setRowHidden(row, !text.contains(filter, Qt::CaseInsensitive));
        }
    }

    void PanelDatabaseDialog::onAccept() {
        const int row = m_table->currentRow();
        if (row < 0) {
            QMessageBox::information(this, QStringLiteral("Panel Database"), QStringLiteral("Select a panel first."));
            return;
        }
        const auto panelIndex = static_cast<std::size_t>(row);
        if (panelIndex >= m_panels.size()) {
            QMessageBox::information(this, QStringLiteral("Panel Database"), QStringLiteral("Select a panel first."));
            return;
        }
        m_selected = m_panels[panelIndex];
        accept();
    }

} // namespace panel_modeler
