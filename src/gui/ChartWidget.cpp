#include "ChartWidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <cmath>

namespace panel_modeler {

    namespace {

        // A matplotlib-tab10-inspired palette, cycled when there are more panels than colors
        const QColor SERIES_COLORS[] = {
            QColor(31, 119, 180),  QColor(255, 127, 14), QColor(44, 160, 44),   QColor(214, 39, 40),
            QColor(148, 103, 189), QColor(140, 86, 75),  QColor(227, 119, 194), QColor(127, 127, 127),
        };
        constexpr int NUM_SERIES_COLORS = sizeof(SERIES_COLORS) / sizeof(SERIES_COLORS[0]);

        constexpr double MARGIN_LEFT = 60.0;
        constexpr double MARGIN_RIGHT = 20.0;
        constexpr double MARGIN_TOP = 20.0;
        constexpr double MARGIN_BOTTOM = 55.0;

        // Rounds the chart's y maximum up to a "nice" value so ticks land on round numbers
        double niceCeiling(const double value) {
            if (value <= 0.0) {
                return 1.0;
            }
            const double magnitude = std::pow(10.0, std::floor(std::log10(value)));
            const double normalized = value / magnitude;
            double nice = 10.0;
            if (normalized <= 1.0) {
                nice = 1.0;
            } else if (normalized <= 2.0) {
                nice = 2.0;
            } else if (normalized <= 2.5) {
                nice = 2.5;
            } else if (normalized <= 5.0) {
                nice = 5.0;
            }
            return nice * magnitude;
        }

    } // namespace

    ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) { setMinimumHeight(260); }

    void ChartWidget::setData(const std::vector<std::vector<double>> &series, const QStringList &names) {
        m_series = series;
        m_names = names;
        update();
    }

    void ChartWidget::clear() {
        m_series.clear();
        m_names.clear();
        update();
    }

    void ChartWidget::paintEvent(QPaintEvent *event) {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), palette().base());

        if (m_series.empty() || m_series.front().size() < 2) {
            painter.setPen(palette().color(QPalette::PlaceholderText));
            painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("Run a simulation to see results"));
            return;
        }

        const double plotWidth = static_cast<double>(width()) - MARGIN_LEFT - MARGIN_RIGHT;
        const double plotHeight = static_cast<double>(height()) - MARGIN_TOP - MARGIN_BOTTOM;
        if (plotWidth <= 0.0 || plotHeight <= 0.0) {
            return;
        }

        double maxValue = 0.0;
        std::size_t maxYear = 0;
        for (const auto &series: m_series) {
            maxYear = std::max(maxYear, series.size() - 1);
            for (const double value: series) {
                maxValue = std::max(maxValue, value);
            }
        }
        const double yMax = niceCeiling(maxValue);

        const auto xForYear = [&](const std::size_t year) {
            return MARGIN_LEFT + plotWidth * static_cast<double>(year) / static_cast<double>(maxYear);
        };
        const auto yForValue = [&](const double value) { return MARGIN_TOP + plotHeight * (1.0 - value / yMax); };

        // gridlines + y tick labels (5 ticks from 0 to yMax)
        painter.setPen(palette().color(QPalette::Text));
        constexpr int NUM_Y_TICKS = 5;
        for (int tick = 0; tick <= NUM_Y_TICKS; ++tick) {
            const double value = yMax * static_cast<double>(tick) / NUM_Y_TICKS;
            const double y = yForValue(value);
            painter.setPen(QPen(palette().color(QPalette::Midlight), 1));
            painter.drawLine(QPointF(MARGIN_LEFT, y), QPointF(MARGIN_LEFT + plotWidth, y));
            painter.setPen(palette().color(QPalette::Text));
            painter.drawText(QRectF(0, y - 9, MARGIN_LEFT - 8, 18), Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(value, 'f', 0));
        }

        // x tick labels (about 10 ticks)
        const std::size_t tickStep = std::max<std::size_t>(1, maxYear / 10);
        for (std::size_t year = 0; year <= maxYear; year += tickStep) {
            painter.drawText(QRectF(xForYear(year) - 25, MARGIN_TOP + plotHeight + 6, 50, 16), Qt::AlignHCenter,
                             QString::number(year));
        }
        painter.drawText(QRectF(MARGIN_LEFT, MARGIN_TOP + plotHeight + 24, plotWidth, 16), Qt::AlignHCenter,
                         QStringLiteral("Year"));
        painter.save();
        painter.translate(14, MARGIN_TOP + plotHeight / 2);
        painter.rotate(-90);
        painter.drawText(QRectF(-60, -8, 120, 16), Qt::AlignCenter, QStringLiteral("Expected power (W)"));
        painter.restore();

        // axes
        painter.setPen(QPen(palette().color(QPalette::Text), 1));
        painter.drawLine(QPointF(MARGIN_LEFT, MARGIN_TOP), QPointF(MARGIN_LEFT, MARGIN_TOP + plotHeight));
        painter.drawLine(QPointF(MARGIN_LEFT, MARGIN_TOP + plotHeight),
                         QPointF(MARGIN_LEFT + plotWidth, MARGIN_TOP + plotHeight));

        // one polyline per panel
        for (std::size_t index = 0; index < m_series.size(); ++index) {
            const auto &series = m_series[index];
            const QColor color = SERIES_COLORS[index % NUM_SERIES_COLORS];
            QPainterPath path;
            for (std::size_t year = 0; year < series.size(); ++year) {
                const QPointF point(xForYear(year), yForValue(series[year]));
                if (year == 0) {
                    path.moveTo(point);
                } else {
                    path.lineTo(point);
                }
            }
            painter.strokePath(path, QPen(color, 2));
        }

        // legend along the top, flowing right-to-left inside the plot area
        double legendX = MARGIN_LEFT + plotWidth - 10.0;
        const double legendY = MARGIN_TOP + 4.0;
        const QFontMetrics metrics = painter.fontMetrics();
        for (int index = static_cast<int>(m_names.size()) - 1; index >= 0; --index) {
            QString label = m_names.at(index);
            if (label.length() > 18) {
                label = label.left(15) + QStringLiteral("...");
            }
            const double labelWidth = static_cast<double>(metrics.horizontalAdvance(label));
            legendX -= labelWidth;
            painter.setPen(palette().color(QPalette::Text));
            painter.drawText(QPointF(legendX, legendY + 10), label);
            legendX -= 18.0;
            painter.setPen(QPen(SERIES_COLORS[static_cast<std::size_t>(index) % NUM_SERIES_COLORS], 3));
            painter.drawLine(QPointF(legendX, legendY + 7), QPointF(legendX + 12, legendY + 7));
            legendX -= 16.0;
        }
    }

} // namespace panel_modeler
