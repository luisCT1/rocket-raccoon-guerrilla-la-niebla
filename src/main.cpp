#include "VentanaJuego.h"

#include <QApplication>
#include <QDir>
#include <QResource>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QResource::registerResource(QCoreApplication::applicationDirPath() + QStringLiteral("/recursos.rcc"));

    VentanaJuego ventana;
    ventana.show();
    return app.exec();
}