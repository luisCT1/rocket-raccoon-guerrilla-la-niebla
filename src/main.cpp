#include <QApplication>
#include "VentanaJuego.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    VentanaJuego ventana;
    ventana.show();
    return app.exec();
}
