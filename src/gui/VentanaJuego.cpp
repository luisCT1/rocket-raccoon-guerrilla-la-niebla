#include "VentanaJuego.h"
#include "EscenaEmboscada.h"

#include <QGraphicsView>
#include <QKeyEvent>
#include <QShowEvent>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

VentanaJuego::VentanaJuego(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Guerrilla de La Niebla"));
    resize(1100, 680);

    escena = new EscenaEmboscada();
    vista = new QGraphicsView(escena);
    vista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setFrameShape(QFrame::NoFrame);
    vista->setRenderHint(QPainter::SmoothPixmapTransform, true);
    vista->setBackgroundBrush(QColor(18, 24, 28));

    ayuda = new QLabel(QStringLiteral(
        "A D o flechas caminan    W salta    Espacio fusil    G granada    Shift impulso    F furia"));
    ayuda->setStyleSheet(QStringLiteral("color:#f2efe6; background:#1b2428; padding:6px;"));

    auto *central = new QWidget(this);
    auto *caja = new QVBoxLayout(central);
    caja->setContentsMargins(0, 0, 0, 0);
    caja->setSpacing(0);
    caja->addWidget(vista, 1);
    caja->addWidget(ayuda);
    setCentralWidget(central);

    reloj = new QTimer(this);
    connect(reloj, &QTimer::timeout, this, &VentanaJuego::tick);
    reloj->start(16);
}

void VentanaJuego::encuadrar() {
    if (!vista || !escena) return;
    vista->fitInView(escena->sceneRect(), Qt::KeepAspectRatio);
}

void VentanaJuego::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    encuadrar();
}

void VentanaJuego::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    encuadrar();
}

void VentanaJuego::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
    escena->pulsar(event->key());
}

void VentanaJuego::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
    escena->soltar(event->key());
}

void VentanaJuego::tick() {
    escena->avanzar(0.016f);
}
