#include "VentanaJuego.h"
#include "EscenaAsalto.h"
#include "EscenaEmboscada.h"
#include "EscenaMision.h"
#include "EscenaPenon.h"
#include "Eventos.h"
#include "MenuInicio.h"

#include <QAudioOutput>
#include <QCursor>
#include <QDir>
#include <QFile>
#include <QGraphicsView>
#include <QMediaPlayer>
#include <QSoundEffect>
#include <QUrl>
#include <algorithm>
#include <cmath>
#include <QPainter>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

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

    menu = new MenuInicio(this);
    connect(menu, &MenuInicio::campana, this, &VentanaJuego::empezarCampana);
    connect(menu, &MenuInicio::nivelElegido, this, &VentanaJuego::empezarUno);
    connect(menu, &MenuInicio::salir, this, &QWidget::close);
    connect(menu, &MenuInicio::pulso, this, &VentanaJuego::tocarClic);

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
    reintentar = new QPushButton(QStringLiteral("Volver a intentarlo"), vista);
    volver->setFocusPolicy(Qt::NoFocus);
    siguiente->setFocusPolicy(Qt::NoFocus);
    reintentar->setFocusPolicy(Qt::NoFocus);
    volver->setStyleSheet(QString::fromLatin1(kBoton));
    siguiente->setStyleSheet(QString::fromLatin1(kBoton));
    reintentar->setStyleSheet(QString::fromLatin1(kBoton));
    volver->hide();
    siguiente->hide();
    reintentar->hide();
    connect(volver, &QPushButton::clicked, this, &VentanaJuego::tocarClic);
    connect(siguiente, &QPushButton::clicked, this, &VentanaJuego::tocarClic);
    connect(reintentar, &QPushButton::clicked, this, &VentanaJuego::tocarClic);
    connect(volver, &QPushButton::clicked, this, &VentanaJuego::volverAlMenu);
    connect(siguiente, &QPushButton::clicked, this, &VentanaJuego::siguienteNivel);
    connect(reintentar, &QPushButton::clicked, this, &VentanaJuego::reintentarNivel);

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
    prepararMusica();
    reloj->start(16);
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

void VentanaJuego::reintentarNivel() {
    abrirNivel(nivelCurso);
}

void VentanaJuego::abrirNivel(int numero) {
    soltarTeclas();
    vista->setScene(nullptr);
    delete escena;
    nivelCurso = numero;
    if (numero == NIVEL_PENON) {
        escena = new EscenaPenon();
        ayuda->setText(QStringLiteral(
            "A D caminan    S se agacha    W salta    Espacio dispara    G granada    Shift impulso    F furia    Esc menú"));
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
            "Vuela el polvorín, tumba la bandera e inutiliza el cañón.\n"
            "Cuando las tres tareas estén hechas, vuelve a la salida. Tienes 150 segundos."));
    } else {
        escena = new EscenaEmboscada();
        ayuda->setText(QStringLiteral(
            "A D caminan    S se agacha    W salta el barranco    Espacio dispara    G granada    Shift impulso    F furia    Esc menú"));
        mision->setText(QStringLiteral(
            "1816 — Emboscada en el camino\n"
            "El convoy real avanza hacia donde empezaste y lleva el aviso.\n"
            "Destruye las tres carretas antes de que lleguen: un disparo al barril las vuela.\n"
            "Si una carreta cruza hasta tu punto de partida, la misión falla."));
    }
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
    reintentar->hide();
    paginas->setCurrentIndex(1);
    reloj->start(16);
    pedirMusica(numero);
    setFocus();
    encuadrar();
    colocarAvisos();
}

void VentanaJuego::prepararMusica() {
    const char *alias[] = {"fondo.mp3", "nivel1.mp3", "nivel2.mp3", "nivel3.mp3"};
    const char *nombre[] = {"fondo.mp3", "nivel1.mp3", "nivel2.mp3", "nivel3.mp3"};
    for (int i = 0; i < 4; ++i) {
        QFile origen(QStringLiteral(":/sonidos/") + QString::fromLatin1(alias[i]));
        if (!origen.open(QIODevice::ReadOnly))
            continue;
        rutasMusica[i] = QDir::temp().filePath(QStringLiteral("niebla-") + QString::fromLatin1(nombre[i]));
        QFile destino(rutasMusica[i]);
        if (!destino.open(QIODevice::WriteOnly)) {
            rutasMusica[i].clear();
            continue;
        }
        destino.write(origen.readAll());
    }
    if (rutasMusica[0].isEmpty())
        return;

    for (int i = 0; i < 2; ++i) {
        voz[i] = new QAudioOutput(this);
        voz[i]->setVolume(0.f);
        tema[i] = new QMediaPlayer(this);
        tema[i]->setAudioOutput(voz[i]);
        tema[i]->setLoops(QMediaPlayer::Infinite);
    }
    pistaDe[0] = 0;
    cargarPista(0, 0);
    tema[0]->play();
    cruceMusica = 0.f;
    fundiendo = false;

    clic = new QSoundEffect(this);
    clic->setSource(QUrl(QStringLiteral("qrc:/sonidos/clic.wav")));
    clic->setVolume(0.46f);
}

void VentanaJuego::cargarPista(int reproductor, int pista) {
    if (reproductor < 0 || reproductor > 1 || !tema[reproductor])
        return;
    if (pista < 0 || pista > 3 || rutasMusica[pista].isEmpty())
        pista = 0;
    tema[reproductor]->setSource(QUrl::fromLocalFile(rutasMusica[pista]));
    tema[reproductor]->setLoops(QMediaPlayer::Infinite);
}

float VentanaJuego::topeMusica(int pista, bool furiosa) const {
    const float base[] = {0.40f, 0.30f, 0.28f, 0.32f};
    if (pista < 0 || pista > 3)
        pista = 0;
    float v = base[pista];
    if (furiosa && pista != 0)
        v += 0.04f;
    return v;
}

void VentanaJuego::pedirMusica(int pista) {
    if (!tema[0] || !tema[1])
        return;
    if (pista < 0 || pista > 3 || rutasMusica[pista].isEmpty())
        pista = 0;
    if (fundiendo && pistaDe[1 - reproductorActivo] == pista)
        return;
    if (fundiendo) {
        const int entra = 1 - reproductorActivo;
        if (cruceMusica < 0.5f) {
            tema[entra]->stop();
            voz[entra]->setVolume(0.f);
        } else {
            tema[reproductorActivo]->stop();
            voz[reproductorActivo]->setVolume(0.f);
            reproductorActivo = entra;
        }
        fundiendo = false;
        cruceMusica = 1.f;
    }
    if (pistaDe[reproductorActivo] == pista)
        return;
    const int entra = 1 - reproductorActivo;
    pistaDe[entra] = pista;
    volumenSalida = voz[reproductorActivo]->volume();
    if (volumenSalida < 0.02f)
        volumenSalida = topeMusica(pistaDe[reproductorActivo], false);
    cargarPista(entra, pista);
    tema[entra]->setPlaybackRate(1.f);
    tema[reproductorActivo]->setPlaybackRate(1.f);
    tema[entra]->play();
    voz[entra]->setVolume(0.f);
    cruceMusica = 0.f;
    fundiendo = true;
}

void VentanaJuego::avanzarMusica(float dt, bool furiosa) {
    if (!tema[0] || !voz[0])
        return;
    if (cruceMusica < 1.f) {
        cruceMusica += dt / 1.25f;
        if (cruceMusica > 1.f)
            cruceMusica = 1.f;
        const float u = cruceMusica * 1.5707963f;
        if (fundiendo) {
            const int entra = 1 - reproductorActivo;
            voz[reproductorActivo]->setVolume(std::cos(u) * volumenSalida);
            voz[entra]->setVolume(std::sin(u) * topeMusica(pistaDe[entra], false));
            if (cruceMusica >= 1.f) {
                tema[reproductorActivo]->stop();
                voz[reproductorActivo]->setVolume(0.f);
                reproductorActivo = entra;
                fundiendo = false;
            }
        } else {
            voz[reproductorActivo]->setVolume(std::sin(u) * topeMusica(pistaDe[reproductorActivo], false));
        }
        return;
    }

    const bool agitar = furiosa && pistaDe[reproductorActivo] != 0;
    const float meta = topeMusica(pistaDe[reproductorActivo], agitar);
    float actual = voz[reproductorActivo]->volume();
    actual += (meta - actual) * std::min(1.f, dt * 3.f);
    voz[reproductorActivo]->setVolume(actual);
    const float ritmo = agitar ? 1.28f : 1.f;
    if (std::fabs(tema[reproductorActivo]->playbackRate() - ritmo) > 0.02f)
        tema[reproductorActivo]->setPlaybackRate(ritmo);
    if (tema[reproductorActivo]->playbackState() != QMediaPlayer::PlayingState)
        tema[reproductorActivo]->play();
}

void VentanaJuego::tocarClic() {
    if (!clic || clic->status() == QSoundEffect::Error)
        return;
    if (clic->isPlaying())
        clic->stop();
    clic->play();
}

void VentanaJuego::volverAlMenu() {
    pedirMusica(0);
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
    reintentar->hide();
    if (menu)
        menu->mostrarRaiz();
    paginas->setCurrentIndex(0);
}

void VentanaJuego::sincronizarTeclas() {
    if (!escena)
        return;
    const int pares[][2] = {
        {0x41, Qt::Key_A}, {0x44, Qt::Key_D}, {0x57, Qt::Key_W}, {0x53, Qt::Key_S},
        {0x25, Qt::Key_Left}, {0x27, Qt::Key_Right}, {0x26, Qt::Key_Up}, {0x28, Qt::Key_Down},
        {0x20, Qt::Key_Space},
    };
    for (const int (&par)[2] : pares) {
        const bool abajo = (GetAsyncKeyState(par[0]) & 0x8000) != 0;
        if (abajo)
            escena->mantener(par[1]);
        else
            escena->soltar(par[1]);
    }
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
    QPushButton *botones[] = {reintentar, siguiente, volver};
    int total = 0;
    int cuantos = 0;
    const int sep = 14;
    for (QPushButton *boton : botones) {
        if (!boton || !boton->isVisible())
            continue;
        boton->adjustSize();
        total += boton->width();
        ++cuantos;
    }
    if (cuantos > 1)
        total += sep * (cuantos - 1);
    int x0 = x + (aviso->width() - total) / 2;
    const int yb = aviso->y() + aviso->height() + 12;
    for (QPushButton *boton : botones) {
        if (!boton || !boton->isVisible())
            continue;
        boton->move(x0, yb);
        boton->raise();
        x0 += boton->width() + sep;
    }
}

void VentanaJuego::mostrarResultado() {
    if (!escena || !escena->acabo() || resultadoVisible)
        return;
    resultadoVisible = true;
    soltarTeclas();
    mision->hide();
    aviso->setText(escena->mensajeFinal());
    aviso->show();
    const bool gano = escena->gano();
    const bool sigue = campana && gano && nivelCurso < NIVEL_PENON;
    reintentar->setVisible(!gano);
    siguiente->setVisible(sigue);
    volver->setVisible(!sigue || !gano);
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
        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && menu && !menu->estaEligiendoNivel()) {
            tocarClic();
            empezarCampana();
        }
        return;
    }
    if (!escena)
        return;
    if (event->key() == Qt::Key_Escape) {
        tocarClic();
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
    avanzarMusica(0.016f, enNivel && escena && escena->enFuria());
    if (!enNivel || !escena)
        return;

    const QPoint local = vista->mapFromGlobal(QCursor::pos());
    if (vista->rect().contains(local)) {
        const QPointF mundo = vista->mapToScene(local);
        escena->apuntar(float(mundo.x()), float(mundo.y()));
    }

    if (!resultadoVisible && !escena->acabo())
        sincronizarTeclas();
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
