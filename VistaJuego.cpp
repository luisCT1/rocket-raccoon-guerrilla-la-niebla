#include "VistaJuego.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <algorithm>
#include <cmath>

#include "AgenteTipos.h"
#include "ArteAsalto.h"
#include "Capitan.h"
#include "Enemigo.h"
#include "Entidad.h"
#include "Estructura.h"
#include "Eventos.h"
#include "Nivel.h"
#include "NivelAsalto.h"
#include "NivelEmboscada.h"
#include "NivelPenon.h"
#include "Proyectil.h"
#include "Rocket.h"
#include "Sonido.h"
#include "Sprites.h"

namespace {
constexpr float ANCHO_VISTA_LATERAL = 1280.f;
constexpr int PERIODO_MS = 16;
constexpr float FPS_SPRITES = 8.f;

QPointF posicionRaton(const QMouseEvent *e) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return e->position();
#else
    return QPointF(e->pos());
#endif
}

QColor colorEstructura(short tipo) {
    switch (tipo) {
    case EST_CARRETA: return QColor(140, 100, 60);
    case EST_BARRIL: return QColor(170, 60, 40);
    case EST_POLVORIN: return QColor(90, 90, 90);
    case EST_BANDERA: return QColor(200, 40, 40);
    case EST_CANON: return QColor(60, 60, 70);
    case EST_TORRE: return QColor(120, 110, 100);
    case EST_PLATAFORMA: return QColor(100, 80, 60);
    case EST_PUENTE: return QColor(150, 120, 80);
    default: return QColor(128, 128, 128);
    }
}

const char *nombreEstructura(short tipo) {
    static const char *nombres[] = {"carreta", "barril", "polvorin", "bandera",
                                    "canon", "torre", "plataforma", "puente"};
    return (tipo >= 0 && tipo < EST_NUM_TIPOS) ? nombres[tipo] : "estructura";
}
}

VistaJuego::VistaJuego(Sprites *sp, Sonido *so, QWidget *padre)
    : QWidget(padre), sprites(sp), sonido(so), estado(JUGANDO), campania(false), botonIzq(false), tAnim(0.f), arte(nullptr),
      esc(1.f), offX(0.f), offY(0.f), visAncho(1280.f), visAlto(720.f) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setMinimumSize(640, 360);
    reloj.setTimerType(Qt::PreciseTimer);
    connect(&reloj, &QTimer::timeout, this, [this] { cuadro(); });
}

VistaJuego::~VistaJuego() { delete arte; }

void VistaJuego::iniciar(short nivel, short dificultad, bool esCampania) {
    juego.iniciarNivel(nivel, dificultad);
    campania = esCampania;
    estado = JUGANDO;
    botonIzq = false;
    teclas.clear();
    eventos.clear();
    limpiarEfectos();
    cronometro.start();
    reloj.start(PERIODO_MS);
    sonido->iniciarMusica();
    setFocus();
}

void VistaJuego::detener() {
    reloj.stop();
    sonido->detenerMusica();
}

void VistaJuego::cuadro() {
    const float dt = static_cast<float>(cronometro.restart()) / 1000.f;
    Nivel *n = juego.nivelActivo();
    if (!n) return;

    if (estado == JUGANDO) {
        procesarEntrada();
        juego.actualizar(dt);
        n->tomarEventos(eventos);
        sonido->procesarEventos(eventos);
        if (juego.getNivelActual() == NIVEL_ASALTO) {
            if (!arte) arte = new ArteAsalto(sprites);
            const QPointF m = aMundo(raton);
            arte->actualizar(static_cast<NivelAsalto *>(n), std::min(dt, 0.05f), static_cast<float>(m.x()),
                             static_cast<float>(m.y()));
        } else {
            detectarEfectos(n);
            for (Efecto &ef : efectos) ef.t += std::min(dt, 0.05f);
            efectos.erase(std::remove_if(efectos.begin(), efectos.end(),
                                         [](const Efecto &ef) { return ef.t >= ef.duracion; }),
                          efectos.end());
        }
        if (n->estaTerminado()) estado = FIN;
    }
    update();
}

void VistaJuego::limpiarEfectos() {
    efectos.clear();
    yaDestruidas.clear();
    if (arte) arte->reiniciar();
}

void VistaJuego::agregarEfecto(float x, float y, float radio, const QString &nombre, float duracion) {
    if (efectos.size() < 64) efectos.push_back({x, y, radio, 0.f, duracion, nombre});
}

void VistaJuego::detectarEfectos(Nivel *n) {
    for (const Entidad *e : n->obtenerEntidades()) {
        if (e->clase() == CLASE_PROYECTIL) {
            const Proyectil *pr = static_cast<const Proyectil *>(e);
            if (pr->yaExplotado() && pr->getRadioExplosion() > 0.f)
                agregarEfecto(pr->getCentroX(), pr->getCentroY(), pr->getRadioExplosion(), "explosion", 0.5f);
        } else if (e->clase() == CLASE_ESTRUCTURA) {
            const Estructura *es = static_cast<const Estructura *>(e);
            if (es->estaDestruida() && !yaDestruidas.contains(es)) {
                yaDestruidas.insert(es);
                agregarEfecto(es->getCentroX(), es->getCentroY(), std::max(es->getAncho(), es->getAlto()) * 0.8f,
                              "explosion", 0.6f);
            }
        }
    }
}

void VistaJuego::dibujarEfectos(QPainter &p) {
    for (const Efecto &ef : efectos) {
        const float avance = ef.t / ef.duracion;
        const QRectF r(ef.x - ef.radio, ef.y - ef.radio, ef.radio * 2.f, ef.radio * 2.f);
        const int n = sprites->contarFrames(ef.nombre);
        const QPixmap *img = n > 0 ? sprites->obtenerAnimado(ef.nombre, ef.t, n / ef.duracion, true)
                                   : sprites->obtener(ef.nombre);
        if (img) {
            p.drawPixmap(r, *img, QRectF(img->rect()));
        } else {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 170, 40, static_cast<int>(200.f * (1.f - avance))));
            const float k = 0.4f + 0.6f * avance;
            p.drawEllipse(QPointF(ef.x, ef.y), ef.radio * k, ef.radio * k);
        }
    }
}

void VistaJuego::procesarEntrada() {
    Nivel *n = juego.nivelActivo();
    const bool cenital = n->esCenital();

    if (teclas.contains(Qt::Key_A) || teclas.contains(Qt::Key_Left)) n->jugadorMueve('L');
    if (teclas.contains(Qt::Key_D) || teclas.contains(Qt::Key_Right)) n->jugadorMueve('R');
    if (cenital) {
        if (teclas.contains(Qt::Key_W) || teclas.contains(Qt::Key_Up)) n->jugadorMueve('U');
        if (teclas.contains(Qt::Key_S) || teclas.contains(Qt::Key_Down)) n->jugadorMueve('D');
    } else if (teclas.contains(Qt::Key_W) || teclas.contains(Qt::Key_Up)) {
        n->jugadorSalta();
    }
    if (botonIzq) {
        const QPointF m = aMundo(raton);
        n->jugadorDispara(static_cast<float>(m.x()), static_cast<float>(m.y()));
    }
}

void VistaJuego::calcularVista() {
    Nivel *n = juego.nivelActivo();
    visAlto = n->getAltoMundo();
    visAncho = (juego.getNivelActual() == NIVEL_EMBOSCADA) ? ANCHO_VISTA_LATERAL : n->getAnchoMundo();
    esc = std::min(width() / visAncho, height() / visAlto);
    offX = (width() - visAncho * esc) * 0.5f;
    offY = (height() - visAlto * esc) * 0.5f;
}

QPointF VistaJuego::aMundo(const QPointF &p) const {
    Nivel *n = juego.nivelActivo();
    return QPointF((p.x() - offX) / esc + n->getCamaraX(), (p.y() - offY) / esc);
}

void VistaJuego::keyPressEvent(QKeyEvent *e) {
    if (e->isAutoRepeat()) return;
    Nivel *n = juego.nivelActivo();
    if (!n) return;
    teclas.insert(e->key());

    switch (e->key()) {
    case Qt::Key_Escape:
        detener();
        emit volverAlMenu();
        return;
    case Qt::Key_P:
        if (estado == JUGANDO) estado = PAUSA;
        else if (estado == PAUSA) { estado = JUGANDO; cronometro.restart(); }
        return;
    default: break;
    }

    if (estado == FIN) {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter || e->key() == Qt::Key_R) {
            if (n->fueGanado() && e->key() != Qt::Key_R) {
                if (campania && juego.getNivelActual() < NIVEL_PENON) {
                    juego.siguienteNivel();
                    limpiarEfectos();
                    estado = JUGANDO;
                    cronometro.restart();
                } else {
                    detener();
                    emit volverAlMenu();
                }
            } else {
                juego.reiniciarNivel();
                limpiarEfectos();
                estado = JUGANDO;
                cronometro.restart();
            }
        }
        return;
    }
    if (estado != JUGANDO) return;

    switch (e->key()) {
    case Qt::Key_Space: n->jugadorSalta(); break;
    case Qt::Key_Shift: n->jugadorDash(); break;
    case Qt::Key_F: n->jugadorActivaFuria(); break;
    case Qt::Key_G: {
        const QPointF m = aMundo(raton);
        n->jugadorLanzaGranada(static_cast<float>(m.x()), static_cast<float>(m.y()));
        break;
    }
    default: break;
    }
}

void VistaJuego::keyReleaseEvent(QKeyEvent *e) {
    if (e->isAutoRepeat()) return;
    teclas.remove(e->key());
}

void VistaJuego::mousePressEvent(QMouseEvent *e) {
    raton = posicionRaton(e);
    Nivel *n = juego.nivelActivo();
    if (!n || estado != JUGANDO) return;
    if (e->button() == Qt::LeftButton) {
        botonIzq = true;
    } else if (e->button() == Qt::RightButton) {
        const QPointF m = aMundo(raton);
        n->jugadorLanzaGranada(static_cast<float>(m.x()), static_cast<float>(m.y()));
    }
}

void VistaJuego::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) botonIzq = false;
}

void VistaJuego::mouseMoveEvent(QMouseEvent *e) { raton = posicionRaton(e); }

void VistaJuego::focusOutEvent(QFocusEvent *e) {
    teclas.clear();
    botonIzq = false;
    if (estado == JUGANDO) estado = PAUSA;
    QWidget::focusOutEvent(e);
}

void VistaJuego::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    Nivel *n = juego.nivelActivo();
    if (!n) return;

    p.setRenderHint(QPainter::SmoothPixmapTransform);
    tAnim = n->tiempoTranscurrido();
    calcularVista();

    p.save();
    p.translate(offX, offY);
    p.scale(esc, esc);
    p.setClipRect(QRectF(0, 0, visAncho, visAlto));
    p.translate(-n->getCamaraX(), 0);
    dibujarMundo(p);
    p.restore();

    dibujarHUD(p, n);

    if (estado == PAUSA) {
        dibujarOverlay(p, "Pausa", "P: continuar    Esc: menú");
    } else if (estado == FIN) {
        if (n->fueGanado()) {
            const bool ultimo = !campania || juego.getNivelActual() >= NIVEL_PENON;
            dibujarOverlay(p, "¡Victoria!", ultimo ? "Enter: volver al menú    R: repetir"
                                                   : "Enter: siguiente nivel    R: repetir");
        } else {
            dibujarOverlay(p, "Derrota", "Enter o R: reintentar    Esc: menú");
        }
    }
}

void VistaJuego::dibujarSprite(QPainter &p, const QString &nombre, const QRectF &r, bool voltear,
                               const QColor &color, bool redondo) {
    const QPixmap *img = sprites->obtenerAnimado(nombre, tAnim, FPS_SPRITES);
    if (img) {
        if (voltear) {
            p.save();
            p.translate(r.center());
            p.scale(-1, 1);
            p.drawPixmap(QRectF(-r.width() / 2, -r.height() / 2, r.width(), r.height()), *img, QRectF(img->rect()));
            p.restore();
        } else {
            p.drawPixmap(r, *img, QRectF(img->rect()));
        }
        return;
    }
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    if (redondo) p.drawEllipse(r);
    else p.drawRect(r);
}

void VistaJuego::dibujarFondo(QPainter &p, Nivel *n) {
    const QRectF mundo(0, 0, n->getAnchoMundo(), n->getAltoMundo());
    const short num = juego.getNivelActual();
    const QPixmap *fondo = sprites->obtener(QString("fondo%1").arg(num));
    if (fondo) {
        p.drawPixmap(mundo, *fondo, QRectF(fondo->rect()));
    } else {
        const QColor tonos[] = {QColor(70, 90, 100), QColor(60, 85, 60), QColor(80, 85, 95)};
        p.fillRect(mundo, tonos[(num - 1) % 3]);
    }

    if (!n->esCenital()) {
        const QColor tierra(70, 55, 40);
        const float y = n->getSuelo();
        if (num == NIVEL_EMBOSCADA) {
            const NivelEmboscada *e = static_cast<const NivelEmboscada *>(n);
            p.fillRect(QRectF(0, y, e->getHuecoInicio(), n->getAltoMundo() - y), tierra);
            p.fillRect(QRectF(e->getHuecoFin(), y, n->getAnchoMundo() - e->getHuecoFin(), n->getAltoMundo() - y), tierra);
        } else {
            p.fillRect(QRectF(0, y, n->getAnchoMundo(), n->getAltoMundo() - y), tierra);
        }
    }
}

void VistaJuego::dibujarMundo(QPainter &p) {
    Nivel *n = juego.nivelActivo();
    if (arte && juego.getNivelActual() == NIVEL_ASALTO) {
        arte->dibujar(p, static_cast<NivelAsalto *>(n));
        return;
    }
    dibujarFondo(p, n);

    const std::vector<Entidad *> &ents = n->obtenerEntidades();
    for (const Entidad *e : ents)
        if (e->estaViva() && e->clase() == CLASE_ESTRUCTURA) dibujarEntidad(p, e);
    dibujarExtras(p, n);
    for (const Entidad *e : ents)
        if (e->estaViva() && e->clase() != CLASE_ESTRUCTURA && e->clase() != CLASE_ROCKET) dibujarEntidad(p, e);
    for (const Entidad *e : ents)
        if (e->estaViva() && e->clase() == CLASE_ROCKET) dibujarEntidad(p, e);
    dibujarEfectos(p);
}

void VistaJuego::dibujarEntidad(QPainter &p, const Entidad *e) {
    const QRectF r(e->getX(), e->getY(), e->getAncho(), e->getAlto());
    const short est = e->estadoVisual();

    switch (e->clase()) {
    case CLASE_ROCKET: {
        const Rocket *rk = static_cast<const Rocket *>(e);
        if (rk->esInvulnerable() && static_cast<int>(juego.nivelActivo()->tiempoTranscurrido() * 12.f) % 2 == 0) {
            p.setOpacity(0.4);
        }
        if (rk->estaFurioso()) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 60, 30, 70));
            p.drawEllipse(r.adjusted(-10, -10, 10, 10));
        }
        static const char *est_n[] = {"quieto", "camina", "aire", "dash"};
        dibujarSprite(p, QString("rocket_") + est_n[est < 0 || est > 3 ? 0 : est], r,
                      rk->getDireccion() == 'L', rk->estaFurioso() ? QColor(220, 90, 40) : QColor(200, 130, 60));
        p.setOpacity(1.0);
        break;
    }
    case CLASE_ENEMIGO: {
        const Enemigo *en = static_cast<const Enemigo *>(e);
        QString nombre = en->getEsCorreo() ? "correo" : (est == 1 ? "soldado_persigue" : "soldado_patrulla");
        dibujarSprite(p, nombre, r, en->getDireccion() == 'L',
                      en->getEsCorreo() ? QColor(230, 200, 60) : QColor(180, 40, 40));
        break;
    }
    case CLASE_CAPITAN: {
        QString nombre = est == 99 ? "capitan_derrotado" : QString("capitan_%1").arg(est);
        const Capitan *c = static_cast<const Capitan *>(e);
        dibujarSprite(p, nombre, r, c->getDireccion() == 'L', QColor(110, 40, 140));
        break;
    }
    case CLASE_PROYECTIL: {
        const Proyectil *pr = static_cast<const Proyectil *>(e);
        QString nombre = pr->esBombeado() ? (pr->esDeRocket() ? "granada" : "bomba")
                                          : (pr->esDeRocket() ? "bala_rocket" : "bala_enemigo");
        dibujarSprite(p, nombre, r, false, pr->esDeRocket() ? QColor(250, 240, 120) : QColor(240, 90, 60), true);
        break;
    }
    case CLASE_ESTRUCTURA: {
        const Estructura *es = static_cast<const Estructura *>(e);
        QString nombre = nombreEstructura(es->getTipo());
        if (es->estaDestruida()) {
            p.setOpacity(0.45);
            nombre += "_ruinas";
        }
        dibujarSprite(p, nombre, r, false, colorEstructura(es->getTipo()));
        p.setOpacity(1.0);
        break;
    }
    default: break;
    }
}

void VistaJuego::dibujarExtras(QPainter &p, Nivel *n) {
    const short num = juego.getNivelActual();
    p.setPen(Qt::NoPen);

    if (num == NIVEL_ASALTO) {
        const NivelAsalto *a = static_cast<const NivelAsalto *>(n);
        p.setBrush(a->tareasCompletas() ? QColor(80, 220, 120, 110) : QColor(255, 255, 255, 40));
        const float sr = a->getSalidaRadio();
        p.drawEllipse(QPointF(a->getSalidaX(), a->getSalidaY()), sr, sr);

        for (const Estructura *t : a->getTorres()) {
            if (t->estaDestruida()) continue;
            const float rc = t->getRadioCono();
            const float ap = t->getApertura();
            const float ang = t->anguloVigilancia();
            p.setBrush(QColor(255, 230, 80, 60));
            const QRectF cono(t->getCentroX() - rc, t->getCentroY() - rc, 2 * rc, 2 * rc);
            const float aGrados = 180.f / 3.14159265f;
            p.drawPie(cono, static_cast<int>(-(ang + ap * 0.5f) * aGrados * 16.f), static_cast<int>(ap * aGrados * 16.f));
        }
    } else if (num == NIVEL_PENON) {
        const NivelPenon *pe = static_cast<const NivelPenon *>(n);
        p.setBrush(pe->estaBanderaPuesta() ? QColor(80, 220, 120, 110) : QColor(255, 255, 255, 40));
        const float br = pe->getBanderaRadio();
        p.drawEllipse(QPointF(pe->getBanderaX(), pe->getBanderaY()), br, br);

        const Capitan *c = pe->getCapitan();
        if (c && !c->estaDerrotado()) {
            if (c->trombaActiva()) {
                const float rt = c->radioTromba();
                const QPixmap *img = sprites->obtenerAnimado("tromba", tAnim, FPS_SPRITES);
                const QRectF area(c->getCentroX() - rt, c->getCentroY() - rt, rt * 2, rt * 2);
                if (img) {
                    p.drawPixmap(area, *img, QRectF(img->rect()));
                } else {
                    p.setBrush(QColor(200, 210, 230, 90));
                    p.drawEllipse(area);
                }
            }
            if (c->giratorioActivo()) {
                float gx = 0.f, gy = 0.f;
                c->posicionGiratorio(gx, gy);
                const QPixmap *img = sprites->obtenerAnimado("giratorio", tAnim, FPS_SPRITES);
                const QRectF area(gx - 22.0, gy - 22.0, 44.0, 44.0);
                if (img) {
                    p.drawPixmap(area, *img, QRectF(img->rect()));
                } else {
                    p.setBrush(QColor(230, 120, 40, 200));
                    p.drawEllipse(area);
                }
            }
        }
    }

    for (const Entidad *e : n->obtenerEntidades()) {
        if (!e->estaViva() || e->clase() != CLASE_PROYECTIL) continue;
        const Proyectil *pr = static_cast<const Proyectil *>(e);
        if (!pr->esBombeado() || pr->yaExplotado()) continue;
        const float rr = pr->getRadioExplosion();
        p.setBrush(pr->esDeRocket() ? QColor(255, 255, 255, 40) : QColor(255, 60, 40, 70));
        p.drawEllipse(QPointF(pr->getDestinoX(), pr->getDestinoY()), rr, rr);
    }
}

void VistaJuego::dibujarHUD(QPainter &p, Nivel *n) {
    Rocket *r = n->jugador();
    p.setRenderHint(QPainter::Antialiasing);
    QFont f = p.font();
    f.setPixelSize(16);
    f.setBold(true);
    p.setFont(f);

    const int x0 = 14;
    int y = 14;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 130));
    p.drawRoundedRect(QRectF(6, 6, 230, 94), 6, 6);

    if (r) {
        p.setPen(Qt::white);
        p.drawText(x0, y + 14, QString("Vidas: %1").arg(r->getVidas()));
        p.drawText(x0, y + 34, QString("Munición: %1    Granadas: %2").arg(r->getMunicion()).arg(r->getGranadas()));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(60, 60, 60));
        p.drawRect(QRectF(x0, y + 46, 200, 12));
        p.setBrush(r->estaFurioso() ? QColor(255, 80, 40) : QColor(230, 160, 40));
        p.drawRect(QRectF(x0, y + 46, 200.0 * std::min(1.f, std::max(0.f, r->getBarraFuria())), 12));
        p.setPen(Qt::white);
        p.drawText(x0, y + 76, r->dashListo() ? "Dash listo" : "Dash en espera");
    }

    QString linea;
    const short num = juego.getNivelActual();
    if (num == NIVEL_EMBOSCADA) {
        const NivelEmboscada *e = static_cast<const NivelEmboscada *>(n);
        linea = e->correoEnFuga() ? "¡El correo huye!" : QString("Carretas restantes: %1").arg(e->getPartesRestantes());
    } else if (num == NIVEL_ASALTO) {
        const NivelAsalto *a = static_cast<const NivelAsalto *>(n);
        linea = QString("Tiempo: %1 s    Tareas: %2/3")
                    .arg(static_cast<int>(std::ceil(a->getTiempoRestante())))
                    .arg(a->getTareasHechas());
    }
    if (!linea.isEmpty()) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 130));
        const int w = p.fontMetrics().horizontalAdvance(linea) + 24;
        p.drawRoundedRect(QRectF(width() - w - 6, 6, w, 32), 6, 6);
        p.setPen(Qt::white);
        p.drawText(QRectF(width() - w - 6, 6, w, 32), Qt::AlignCenter, linea);
    }

    if (num == NIVEL_PENON) {
        const NivelPenon *pe = static_cast<const NivelPenon *>(n);
        const Capitan *c = pe->getCapitan();
        if (c) {
            const float frac = std::max(0.f, c->getVida() / c->getVidaMax());
            const double w = std::min(520.0, width() * 0.6);
            const double x = (width() - w) / 2.0, yb = height() - 34.0;
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, 150));
            p.drawRoundedRect(QRectF(x - 6, yb - 24, w + 12, 40), 6, 6);
            p.setPen(Qt::white);
            p.drawText(QRectF(x, yb - 22, w, 18), Qt::AlignCenter, QString("Capitán Nubarrón   Fase %1").arg(c->getFase()));
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(60, 60, 60));
            p.drawRect(QRectF(x, yb, w, 10));
            p.setBrush(QColor(200, 50, 50));
            p.drawRect(QRectF(x, yb, w * frac, 10));
        }
    }
}

void VistaJuego::dibujarOverlay(QPainter &p, const QString &titulo, const QString &subtitulo) {
    p.fillRect(rect(), QColor(0, 0, 0, 150));
    p.setPen(Qt::white);
    QFont f = p.font();
    f.setPixelSize(54);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRectF(0, height() * 0.35, width(), 70), Qt::AlignCenter, titulo);
    f.setPixelSize(20);
    f.setBold(false);
    p.setFont(f);
    p.drawText(QRectF(0, height() * 0.35 + 80, width(), 30), Qt::AlignCenter, subtitulo);
}
