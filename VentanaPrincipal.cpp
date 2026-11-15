#include "VentanaPrincipal.h"

#include <QMessageBox>
#include <exception>

#include "MenuWidget.h"
#include "Sonido.h"
#include "Sprites.h"
#include "VistaJuego.h"

VentanaPrincipal::VentanaPrincipal() {
    setWindowTitle("Rocket Raccoon en la Guerrilla de la Niebla");
    resize(1280, 720);

    const QString base = QString::fromUtf8(RUTA_RECURSOS);
    sprites = new Sprites(base + "sprites/");
    sonido = new Sonido(base + "audio/", this);

    menu = new MenuWidget;
    vista = new VistaJuego(sprites, sonido);
    addWidget(menu);
    addWidget(vista);

    connect(menu, &MenuWidget::jugar, this, [this](short nivel, short dif, bool campania) {
        try {
            vista->iniciar(nivel, dif, campania);
            setCurrentWidget(vista);
            vista->setFocus();
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", QString("No se pudo iniciar el nivel:\n") + e.what());
        }
    });
    connect(menu, &MenuWidget::salir, this, &QWidget::close);
    connect(vista, &VistaJuego::volverAlMenu, this, [this] { mostrarMenu(); });
}

VentanaPrincipal::~VentanaPrincipal() { delete sprites; }

void VentanaPrincipal::mostrarMenu() { setCurrentWidget(menu); }
