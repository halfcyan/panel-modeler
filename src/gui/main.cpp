#include <QApplication>

#include "config.h"
#include "gui/MainWindow.h"

int main(int argc, char *argv[]) {
    const QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("panel-modeler"));
    QCoreApplication::setApplicationName(QStringLiteral("panel-modeler"));
    QCoreApplication::setApplicationVersion(QStringLiteral(PANEL_MODELER_VERSION));

    panel_modeler::MainWindow window;
    window.show();
    return QApplication::exec();
}
