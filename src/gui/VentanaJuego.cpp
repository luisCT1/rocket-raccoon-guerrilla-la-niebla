#include "VentanaJuego.h"

#include <QKeyEvent>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "Juego.h"
#include "Nivel.h"
#include "NivelDemo.h"
#include "Rocket.h"
#include "Proyectil.h"

VentanaJuego::VentanaJuego(QWidget *parent)
    : QMainWindow(parent), juego(new Juego), reloj(new QTimer(this)), hud(new QLabel) {
    setWindowTitle(QStringLiteral("Guerrilla de La Niebla (1816)"));
    resize(720, 240);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    hud->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    hud->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(hud);
    setCentralWidget(central);

    //Arrancamos el demo de combate; después aquí irá el nivel 1 de verdad
    juego->iniciarNivel(0, 1);

    connect(reloj, &QTimer::timeout, this, &VentanaJuego::tick);
    reloj->start(16);

    actualizarHud();
}

VentanaJuego::~VentanaJuego() {
    delete juego;
    juego = nullptr;
}

Rocket *VentanaJuego::rocketActivo() const {
    if (!juego || !juego->nivelActivo()) return nullptr;
    return juego->nivelActivo()->jugador();
}

void VentanaJuego::keyPressEvent(QKeyEvent *event) {
    teclas.insert(event->key());

    Rocket *r = rocketActivo();
    if (!r) return;

    if (event->key() == Qt::Key_Space) {
        Proyectil *p = r->disparar(1.f, -0.15f);
        if (p && juego->nivelActivo()) {
            juego->nivelActivo()->agregarEntidad(p);
        }
    }
    if (event->key() == Qt::Key_G) {
        Proyectil *p = r->lanzarGranada(0.85f, -0.75f);
        if (p && juego->nivelActivo()) {
            juego->nivelActivo()->agregarEntidad(p);
        }
    }
    if (event->key() == Qt::Key_Shift) {
        r->dash('R');
    }
    if (event->key() == Qt::Key_F) {
        r->activarFuria();
    }

    QMainWindow::keyPressEvent(event);
}

void VentanaJuego::keyReleaseEvent(QKeyEvent *event) {
    teclas.remove(event->key());
    QMainWindow::keyReleaseEvent(event);
}

void VentanaJuego::tick() {
    Rocket *r = rocketActivo();
    if (r) {
        if (teclas.contains(Qt::Key_Left) || teclas.contains(Qt::Key_A)) r->mover('L');
        if (teclas.contains(Qt::Key_Right) || teclas.contains(Qt::Key_D)) r->mover('R');
        if (teclas.contains(Qt::Key_Up) || teclas.contains(Qt::Key_W)) r->saltar();
    }

    if (juego) {
        juego->actualizar(0.016f);
    }
    actualizarHud();
}

void VentanaJuego::actualizarHud() {
    Nivel *n = juego ? juego->nivelActivo() : nullptr;
    Rocket *r = n ? n->jugador() : nullptr;
    auto *demo = dynamic_cast<NivelDemo *>(n);

    QString texto;
    texto += QStringLiteral("Guerrilla de La Niebla — capa lógica en prueba\n\n");
    texto += QStringLiteral("A/D o flechas mover | W saltar | Espacio disparar\n");
    texto += QStringLiteral("G granada | Shift dash | F furia\n\n");

    if (!r) {
        texto += QStringLiteral("No hay Rocket cargado.");
        hud->setText(texto);
        return;
    }

    texto += QStringLiteral("Rocket  x=%1  y=%2  vidas=%3  municion=%4  furia=%5\n")
                 .arg(r->getX(), 0, 'f', 1)
                 .arg(r->getY(), 0, 'f', 1)
                 .arg(r->getVidas())
                 .arg(r->getMunicion())
                 .arg(r->estaFurioso() ? QStringLiteral("SI") : QStringLiteral("NO"));

    if (demo) {
        texto += QStringLiteral("Enemigos derrotados: %1 / 5\n").arg(demo->getEnemigosDerrotados());
    }

    if (n) {
        if (n->victoria()) texto += QStringLiteral("\nVictoria\n");
        if (n->derrota()) texto += QStringLiteral("\nDerrota\n");
    }

    hud->setText(texto);
}
