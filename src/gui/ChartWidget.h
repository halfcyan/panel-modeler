#ifndef PANEL_MODELER_GUI_CHARTWIDGET_H
#define PANEL_MODELER_GUI_CHARTWIDGET_H

#include <QStringList>
#include <QWidget>

#include <vector>

namespace panel_modeler {

    // Minimal line chart of simulation results (power per panel over the years),
    // painted directly with QPainter so we do not need the Qt Charts module.
    class ChartWidget : public QWidget {
        Q_OBJECT
    public:
        explicit ChartWidget(QWidget *parent = nullptr);

        // Series are indexed by [panel][year]; names has one label per panel.
        void setData(const std::vector<std::vector<double>> &series, const QStringList &names);
        void clear();

    protected:
        void paintEvent(QPaintEvent *event) override;

    private:
        std::vector<std::vector<double>> m_series;
        QStringList m_names;
    };

} // namespace panel_modeler

#endif // PANEL_MODELER_GUI_CHARTWIDGET_H
