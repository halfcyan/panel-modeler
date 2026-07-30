#ifndef PANEL_MODELER_GUI_PANELDATABASEDIALOG_H
#define PANEL_MODELER_GUI_PANELDATABASEDIALOG_H

#include <QDialog>

#include <optional>
#include <vector>

#include "net/PanelDatabase.h"

class QComboBox;
class QLineEdit;
class QPushButton;
class QTableWidget;

namespace panel_modeler {

    // Browse panel specifications: either the bundled/installed database or a
    // self-hosted remote one. Accepting the dialog adds the selected panel.
    class PanelDatabaseDialog : public QDialog {
        Q_OBJECT
    public:
        explicit PanelDatabaseDialog(QWidget *parent = nullptr);

        // The panel the user picked; only meaningful after exec() == Accepted.
        [[nodiscard]] std::optional<PanelSpec> selectedPanel() const;

    private:
        void loadFromCurrentSource();
        void populate(const std::vector<PanelSpec> &panels, const QString &source);
        void applyFilter();
        void onAccept();

        QComboBox *m_sourceCombo;
        QLineEdit *m_urlEdit;
        QPushButton *m_reloadButton;
        QLineEdit *m_filterEdit;
        QTableWidget *m_table;
        PanelDatabase *m_database;
        std::vector<PanelSpec> m_panels;
        std::optional<PanelSpec> m_selected;
    };

} // namespace panel_modeler

#endif // PANEL_MODELER_GUI_PANELDATABASEDIALOG_H
