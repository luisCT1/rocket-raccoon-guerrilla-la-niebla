#include "VentanaJuego.h"
#include "EscenaAsalto.h"
#include "EscenaEmboscada.h"
#include "EscenaMision.h"
#include "EscenaPenon.h"
#include "Eventos.h"
#include "MenuInicio.h"
#include "Sonido.h"

#include <QCursor>
#include <QGraphicsView>
#include <QPainter>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace {
const char *kBoton =
    "QPushButton { color:#f6f1e4; background:#3e5c45; border:2px solid #c7a15a; "
    "border-radius:6px; padding:8px 18px; font-family: Georgia; font-size:16px; }"
    "QPushButton:hover { background:#52785c; }";

const char *kCartel =
    "color:#f6f1e4; background:rgba(22,32,24,230); border:2px solid #c7a15a; "
    "border-radius:8px; padding:18px 22px; font-size:26px; font-weight:bold; font-family: Georgia;";
}

VentanaJuego::VentanaJuego(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Guerrilla de La Niebla"));
    resize(1280, 760);

    sonido = new Sonido(this);

    menu = new MenuInicio(this);
    connect(menu, &MenuInicio::campana, this, &VentanaJuego::empezarCampana);
    connect(menu, &MenuInicio::nivelElegido, this, &VentanaJuego::empezarUno);
    connect(menu, &MenuInicio::salir, this, &QWidget::close);
    connect(menu, &MenuInicio::campana, this, [this] { sonido->tocar(QStringLiteral("clic")); });
    connect(menu, &MenuInicio::nivelElegido, this, [this](int) { sonido->tocar(QStringLiteral("clic")); });
    connect(menu, &MenuInicio::dificultadCambiada, this, [this] { sonido->tocar(QStringLiteral("clic")); });

    vista = new QGraphicsView();
    vista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setFrameShape(QFrame::NoFrame);
    vista->setRenderHint(QPainter::SmoothPixmapTransform, false);
    vista->setBackgroundBrush(QColor(168, 186, 190));
    vista->setFocusPolicy(Qt::NoFocus);
    vista->setInteractive(false);
    vista->setMouseTracking(true);

    ayuda = new QLabel(QStringLiteral(
        "A D caminan    W salta    Espacio dispara    G granada    Shift impulso    F furia    Esc menú"));
    ayuda->setStyleSheet(QStringLiteral("color:#d7e0d6; background:#1b2420; padding:6px 8px; font-family: Georgia;"));

    mision = new QLabel(vista);
    mision->setWordWrap(true);
    mision->setAlignment(Qt::AlignCenter);
    mision->setStyleSheet(QString::fromLatin1(kCartel).replace(QStringLiteral("font-size:26px"), QStringLiteral("font-size:18px")));
    mision->setFixedWidth(720);
    mision->hide();

    aviso = new QLabel(vista);
    aviso->setWordWrap(true);
    aviso->setAlignment(Qt::AlignCenter);
    aviso->setStyleSheet(QString::fromLatin1(kCartel));
    aviso->setFixedWidth(640);
    aviso->hide();

    volver = new QPushButton(QStringLiteral("Volver al menú"), vista);
    siguiente = new QPushButton(QStringLiteral("Siguiente nivel"), vista);
    volver->setFocusPolicy(Qt::NoFocus);
    siguiente->setFocusPolicy(Qt::NoFocus);
    volver->setStyleSheet(QString::fromLatin1(kBoton));
    siguiente->setStyleSheet(QString::fromLatin1(kBoton));
    volver->hide();
    siguiente->hide();
    connect(volver, &QPushButton::clicked, this, &VentanaJuego::volverAlMenu);
    connect(siguiente, &QPushButton::clicked, this, &VentanaJuego::siguienteNivel);
    connect(volver, &QPushButton::clicked, this, [this] { sonido->tocar(QStringLiteral("clic")); });
    connect(siguiente, &QPushButton::clicked, this, [this] { sonido->tocar(QStringLiteral("clic")); });

    auto *nivel = new QWidget();
    auto *caja = new QVBoxLayout(nivel);
    caja->setContentsMargins(0, 0, 0, 0);
    caja->setSpacing(0);
    caja->addWidget(vista, 1);
    caja->addWidget(ayuda);

    paginas = new QStackedWidget(this);
    paginas->addWidget(menu);
    paginas->addWidget(nivel);
    setCentralWidget(paginas);

    reloj = new QTimer(this);
    connect(reloj, &QTimer::timeout, this, &VentanaJuego::tick);

    sonido->iniciarMenu();
}

VentanaJuego::~VentanaJuego() {
    if (vista)
        vista->setScene(nullptr);
    delete escena;
    escena = nullptr;
}

void VentanaJuego::empezarCampana() {
    campana = true;
    abrirNivel(NIVEL_EMBOSCADA);
}

void VentanaJuego::empezarUno(int numero) {
    campana = false;
    abrirNivel(numero);
}

void VentanaJuego::siguienteNivel() {
    if (nivelCurso < NIVEL_PENON)
        abrirNivel(nivelCurso + 1);
}

void VentanaJuego::abrirNivel(int numero) {
    soltarTeclas();
    vista->setScene(nullptr);
    delete escena;
    nivelCurso = numero;
    const short dificultad = menu->dificultad();
    if (numero == NIVEL_PENON) {
        escena = new EscenaPenon(dificultad);
        ayuda->setText(QStringLiteral(
            "A D caminan    W salta    Espacio dispara    G granada    Shift impulso    F furia    Esc menú"));
        mision->setText(QStringLiteral(
            "1816 — Duelo en el Peñón\n"
            "El aviso no llegó. El capitán te espera en la roca, con el cañón cargado.\n"
            "Cruza las losas que suben y bajan. Cuando caiga, planta la bandera en la cima."));
    } else if (numero == NIVEL_ASALTO) {
        escena = new EscenaAsalto();
        ayuda->setText(QStringLiteral(
            "W A S D se mueven    Espacio dispara    G granada    Shift impulso    F furia    Esc menú"));
        mision->setText(QStringLiteral(
            "1816 — Asalto al patio\n"
            "Vuela el polvorín, tumba la bandera e inutiliza el cañón: está tras barreras, ataca por el pasillo del oeste.\n"
            "El cañón siempre te alcanza, no te quedes quieto. Los barriles rojos explotan en cadena y un soldado o torre que te vea sube la alarma.\n"
            "Con las tres tareas hechas salta la alarma maxima y los soldados no dejan de entrar por la puerta sur: aguanta 20 segundos hasta que lleguen los patriotas y ganas. Para las tareas tienes 120 segundos."));
    } else {
        escena = new EscenaEmboscada();
        ayuda->setText(QStringLiteral(
            "A D caminan    W salta el barranco    Espacio dispara    G granada    Shift impulso    F furia    Esc menú"));
        mision->setText(QStringLiteral(
            "1816 — Emboscada en el camino\n"
            "El convoy real lleva el aviso. Destruye las tres carretas: un disparo al barril las vuela.\n"
            "Si el correo llega al final del camino, la misión falla. Tienes tres oportunidades."));
    }
    if (numero == NIVEL_PENON)
        mision->setText(mision->text() + (dificultad == DIF_DIFICIL
            ? QStringLiteral("\nDificultad: difícil. Un Capitán más fuerte que levanta un escudo al llegar a la mitad de su vida, y un cañón que apunta adelantándose a tus pasos. Las losas oscilan más. Una roca te deja subir durante el combate.")
            : QStringLiteral("\nDificultad: normal. Una roca te deja subir durante el combate.")));
    escena->fijarSonido(sonido);
    sonido->iniciarNivel(numero);
    vista->setBackgroundBrush(escena->backgroundBrush());
    vista->setScene(escena);
    enNivel = true;
    resultadoVisible = false;
    tiempoMision = 0.f;
    mision->adjustSize();
    mision->show();
    mision->raise();
    aviso->hide();
    volver->hide();
    siguiente->hide();
    paginas->setCurrentIndex(1);
    reloj->start(16);
    setFocus();
    encuadrar();
    colocarAvisos();
}

void VentanaJuego::volverAlMenu() {
    reloj->stop();
    soltarTeclas();
    enNivel = false;
    campana = false;
    resultadoVisible = false;
    vista->setScene(nullptr);
    delete escena;
    escena = nullptr;
    mision->hide();
    aviso->hide();
    volver->hide();
    siguiente->hide();
    paginas->setCurrentIndex(0);
    sonido->iniciarMenu();
}

void VentanaJuego::soltarTeclas() {
    if (!escena)
        return;
    const int teclas[] = {Qt::Key_A, Qt::Key_D, Qt::Key_W, Qt::Key_S, Qt::Key_Left, Qt::Key_Right,
                          Qt::Key_Up, Qt::Key_Down, Qt::Key_Space};
    for (int tecla : teclas)
        escena->soltar(tecla);
}

void VentanaJuego::encuadrar() {
    if (!vista || !escena)
        return;
    const QSize area = vista->viewport()->size();
    if (area.width() > 0 && area.height() > 0)
        escena->fijarProporcion(float(area.width()) / float(area.height()));
    vista->fitInView(escena->rectCamara(), Qt::KeepAspectRatioByExpanding);
}

void VentanaJuego::colocarAvisos() {
    if (!vista)
        return;
    if (mision && mision->isVisible()) {
        mision->move((vista->width() - mision->width()) / 2, 24);
        mision->raise();
    }
    if (!aviso || !aviso->isVisible())
        return;
    aviso->setFixedWidth(qMin(680, qMax(280, vista->width() - 80)));
    aviso->adjustSize();
    const int x = (vista->width() - aviso->width()) / 2;
    const int y = qMax(48, (vista->height() - aviso->height()) / 2 - 28);
    aviso->move(x, y);
    aviso->raise();
    QPushButton *boton = siguiente && siguiente->isVisible() ? siguiente : volver;
    if (boton && boton->isVisible()) {
        boton->adjustSize();
        boton->move(x + (aviso->width() - boton->width()) / 2, aviso->y() + aviso->height() + 12);
        boton->raise();
    }
}

void VentanaJuego::mostrarResultado() {
    if (!escena || !escena->acabo() || resultadoVisible)
        return;
    resultadoVisible = true;
    sonido->detenerMusica();
    mision->hide();
    aviso->setText(escena->mensajeFinal());
    aviso->show();
    const bool sigue = campana && escena->gano() && nivelCurso < NIVEL_PENON;
    siguiente->setVisible(sigue);
    volver->setVisible(!sigue);
    colocarAvisos();
}

void VentanaJuego::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    encuadrar();
    colocarAvisos();
}

void VentanaJuego::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat())
        return;
    if (!enNivel) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
            empezarCampana();
        return;
    }
    if (!escena)
        return;
    if (event->key() == Qt::Key_Escape) {
        volverAlMenu();
        return;
    }
    if (resultadoVisible)
        return;
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
    encuadrar();

    if (mision->isVisible()) {
        tiempoMision += 0.016f;
        if (tiempoMision > 8.f || escena->acabo())
            mision->hide();
    }

    if (escena->acabo())
        mostrarResultado();
    colocarAvisos();
}
