#include "VentanaJuego.h"
#include "EscenaEmboscada.h"
#include "MenuInicio.h"

#include <QCursor>
#include <QGraphicsView>
#include <QPainter>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

VentanaJuego::VentanaJuego(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Guerrilla de La Niebla"));
    resize(1280, 760);

    menu = new MenuInicio(this);
    connect(menu, &MenuInicio::jugar, this, &VentanaJuego::empezarNivel);
    connect(menu, &MenuInicio::salir, this, &QWidget::close);

    vista = new QGraphicsView();
    vista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setFrameShape(QFrame::NoFrame);
    vista->setRenderHint(QPainter::SmoothPixmapTransform, false);
    vista->setBackgroundBrush(QColor(168, 186, 190));
    vista->setFocusPolicy(Qt::NoFocus);
    vista->setInteractive(false);
    vista->setMouseTracking(true);

    hud = new QLabel();
    hud->setStyleSheet(QStringLiteral("color:#f6f1e4; background:#1b2420; padding:8px; font-size:15px;"));

    ayuda = new QLabel(QStringLiteral(
        "Misión: destruye las 3 carretas y no dejes escapar al correo.    "
        "A D caminan    W salta el barranco    Espacio dispara    G granada    Shift impulso    F furia    Esc menú"));
    ayuda->setStyleSheet(QStringLiteral("color:#d7e0d6; background:#1b2420; padding:4px 8px 8px 8px;"));

    mision = new QLabel(vista);
    mision->setWordWrap(true);
    mision->setAlignment(Qt::AlignCenter);
    mision->setText(QStringLiteral(
        "1816 — Emboscada en el camino\n"
        "El convoy real lleva el aviso. Destruye las tres carretas: un disparo al barril las vuela.\n"
        "Si el correo llega al final del camino, la misión falla. Tienes tres oportunidades."));
    mision->setStyleSheet(QStringLiteral(
        "color:#f6f1e4; background:rgba(22,32,24,210); border:2px solid #c7a15a; "
        "border-radius:8px; padding:14px; font-size:18px;"));
    mision->setFixedWidth(720);
    mision->hide();

    aviso = new QLabel();
    aviso->setAlignment(Qt::AlignCenter);
    aviso->setStyleSheet(QStringLiteral("color:#f3e6c4; background:#243028; padding:16px; font-size:26px; font-weight:bold;"));
    aviso->hide();

    volver = new QPushButton(QStringLiteral("Volver al menú"));
    volver->setFocusPolicy(Qt::NoFocus);
    volver->setStyleSheet(QStringLiteral(
        "QPushButton { color:#f6f1e4; background:#3e5c45; border:2px solid #c7a15a; "
        "border-radius:6px; padding:8px 16px; } QPushButton:hover { background:#52785c; }"));
    volver->hide();
    connect(volver, &QPushButton::clicked, this, &VentanaJuego::volverAlMenu);

    auto *nivel = new QWidget();
    auto *caja = new QVBoxLayout(nivel);
    caja->setContentsMargins(0, 0, 0, 0);
    caja->setSpacing(0);
    caja->addWidget(hud);
    caja->addWidget(vista, 1);
    caja->addWidget(aviso);
    caja->addWidget(volver, 0, Qt::AlignHCenter);
    caja->addWidget(ayuda);

    paginas = new QStackedWidget(this);
    paginas->addWidget(menu);
    paginas->addWidget(nivel);
    setCentralWidget(paginas);

    reloj = new QTimer(this);
    connect(reloj, &QTimer::timeout, this, &VentanaJuego::tick);
}

VentanaJuego::~VentanaJuego() {
    if (vista)
        vista->setScene(nullptr);
    delete escena;
    escena = nullptr;
}

void VentanaJuego::empezarNivel() {
    soltarTeclas();
    vista->setScene(nullptr);
    delete escena;
    escena = new EscenaEmboscada();
    vista->setBackgroundBrush(escena->backgroundBrush());
    vista->setScene(escena);
    enNivel = true;
    tiempoMision = 0.f;
    mision->adjustSize();
    mision->move((vista->width() - mision->width()) / 2, 18);
    mision->show();
    mision->raise();
    aviso->hide();
    volver->hide();
    paginas->setCurrentIndex(1);
    reloj->start(16);
    setFocus();
    encuadrar();
}

void VentanaJuego::volverAlMenu() {
    reloj->stop();
    soltarTeclas();
    enNivel = false;
    vista->setScene(nullptr);
    delete escena;
    escena = nullptr;
    mision->hide();
    aviso->hide();
    volver->hide();
    paginas->setCurrentIndex(0);
}

void VentanaJuego::soltarTeclas() {
    if (!escena)
        return;
    escena->soltar(Qt::Key_A);
    escena->soltar(Qt::Key_D);
    escena->soltar(Qt::Key_Left);
    escena->soltar(Qt::Key_Right);
    escena->soltar(Qt::Key_Space);
}

void VentanaJuego::encuadrar() {
    if (!vista || !escena)
        return;
    vista->fitInView(escena->rectCamara(), Qt::KeepAspectRatioByExpanding);
}

void VentanaJuego::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    encuadrar();
}

void VentanaJuego::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;
    if (!enNivel) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
            empezarNivel();
        return;
    }
    if (!escena)
        return;
    if (event->key() == Qt::Key_Escape) {
        volverAlMenu();
        return;
    }
    escena->pulsar(event->key());
}

void VentanaJuego::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat() || !escena)
        return;
    escena->soltar(event->key());
}

void VentanaJuego::tick() {
    if (!enNivel || !escena)
        return;

    const QPoint local = vista->mapFromGlobal(QCursor::pos());
    if (vista->rect().contains(local)) {
        const QPointF mundo = vista->mapToScene(local);
        escena->apuntar(float(mundo.x()), float(mundo.y()));
    }

    escena->avanzar(0.016f);
    hud->setText(escena->textoHud());
    encuadrar();

    if (mision->isVisible()) {
        tiempoMision += 0.016f;
        mision->move((vista->width() - mision->width()) / 2, 18);
        mision->raise();
        if (tiempoMision > 8.f || escena->acabo())
            mision->hide();
    }

    if (escena->acabo()) {
        aviso->setText(escena->mensajeFinal());
        aviso->show();
        volver->show();
    }
}
