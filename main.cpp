#include <QApplication>
#include <QMessageBox>
#include <exception>

#include "VentanaPrincipal.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    try {
        VentanaPrincipal ventana;
        ventana.show();
        return app.exec();
    } catch (const std::exception &e) {
        QMessageBox::critical(nullptr, "Error", e.what());
        return 1;
    }
}
