#include "EscenaAsalto.h"

#include "Enemigo.h"
#include "Estructura.h"
#include "Eventos.h"
#include "Juego.h"
#include "NivelAsalto.h"
#include "Proyectil.h"
#include "Rocket.h"
#include "Sonido.h"

#include <QColor>
#include <QFont>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPoint>
#include <QPolygonF>
#include <QRadialGradient>
#include <QRect>
#include <QTransform>

#include <algorithm>
#include <cmath>

namespace {
namespace Atlas {
constexpr int ROCKET_ANCHO_CELDA = 192;
constexpr int ROCKET_ALTO_CELDA = 136;
constexpr int ROCKET_PIE_X = 89;
constexpr int ROCKET_PIE_Y = 129;
constexpr int ROCKET_COLUMNAS = 6;
constexpr int ROCKET_FILAS = 11;
constexpr float ESCALA_ROCKET = 0.70f;
constexpr int SOLDADO_ANCHO_CELDA = 192;
constexpr int SOLDADO_ALTO_CELDA = 160;
constexpr int SOLDADO_PIE_X = 63;
constexpr int SOLDADO_PIE_Y = 154;
constexpr int SOLDADO_COLUMNAS = 6;
constexpr int SOLDADO_FILAS = 10;
constexpr float ESCALA_SOLDADO = 0.60f;

enum FilaRocket {
    ROCKET_CAMINA_1 = 1,
    ROCKET_CAMINA_DER = 3,
    ROCKET_DISPARA = 4,
    ROCKET_DISPARA_Y_GRANADA = 5,
    ROCKET_DASH = 7,
    ROCKET_DANIO = 8,
    ROCKET_REPOSO = 10
};
enum FilaSoldado {
    SOLDADO_CAMINA_2 = 2, SOLDADO_CAMINA_3 = 3,
    SOLDADO_DISPARA_0 = 4,
    SOLDADO_DISPARA_1 = 5,
    SOLDADO_CAE = 7,
    SOLDADO_CAIDO = 8,
    SOLDADO_POSES = 9
};

const QRect TORRE_A(47, 20, 129, 239);
const QRect TORRE_B(425, 20, 126, 239);
const QRect CANON_2(1206, 263, 134, 117);
const QRect CAJON_BALAS(126, 444, 115, 94);
const QRect BARRIL_ROJO(516, 449, 58, 85);
const QRect SACOS_2(947, 449, 174, 69);
const QRect BANDERA_REAL_ROJA(1060, 550, 67, 139);
const QRect HUMO_1(745, 861, 119, 127);
const QRect HUMO_2(899, 881, 94, 105);
const QRect EXPLOSION_1(1027, 856, 153, 145);
const QRect EXPLOSION_2_FUEGO(1215, 840, 118, 161);
const QRect EXPLOSION_3(1365, 845, 156, 156);

}
}

namespace {
const float kVistaAncho = 1280.f;
const float kVistaAlto = 720.f;
const float kPi = 3.14159265f;
const float kZCono = 2.3f;
const float kZAviso = 2.35f;
const float kZCuerpo = 2.4f;
const float kZSombra = 2.5f;
const float kZPersonaje = 3.f;
const float kZHumo = 3.95f;
const float kZProyectil = 4.2f;
const float kZEfecto = 5.f;
const float kZFlecha = 6.f;
const float kZHud = 40.f;
const float kDisparoSoldado = 0.30f;
const float kDisparoRocket = 0.22f;
const float kLanzaRocket = 0.45f;
const float kDanioRocket = 0.40f;
const float kCaidaDuracion = 1.9f;
const float kExplosionDuracion = 1.4f;
const float kPanelAncho = 296.f;
const float kMargen = 16.f;
const int kMaxEfectos = 64;
const float kBarreraAncho = 120.f;

float limitar(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

QPixmap recorte(const QPixmap &hoja, const QRect &zona) {
    if (hoja.isNull())
        return QPixmap();
    return hoja.copy(zona.intersected(hoja.rect()));
}

QPixmap escalar(const QPixmap &imagen, float factor) {
    if (imagen.isNull())
        return imagen;
    return imagen.scaled(qMax(1, qRound(imagen.width() * factor)), qMax(1, qRound(imagen.height() * factor)),
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QPixmap espejar(const QPixmap &imagen) {
    if (imagen.isNull())
        return imagen;
    return imagen.transformed(QTransform().scale(-1, 1));
}

QPixmap oscurecer(const QPixmap &imagen, const QColor &tono) {
    if (imagen.isNull())
        return imagen;
    QPixmap salida = imagen.copy();
    {
        QPainter p(&salida);
        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(salida.rect(), tono);
    }
    return salida;
}

QPixmap bloque(int ancho, int alto, const QColor &color) {
    QPixmap px(ancho, alto);
    px.fill(Qt::transparent);
    {
        QPainter p(&px);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(color.darker(160), 2));
        p.setBrush(color);
        p.drawRoundedRect(QRectF(1, 1, ancho - 2, alto - 2), 6, 6);
    }
    return px;
}

QPixmap esfera(int lado, const QColor &centro, const QColor &borde) {
    QPixmap px(lado, lado);
    px.fill(Qt::transparent);
    {
        QPainter p(&px);
        p.setRenderHint(QPainter::Antialiasing);
        QRadialGradient degradado(lado * 0.4, lado * 0.36, lado * 0.62);
        degradado.setColorAt(0.0, centro);
        degradado.setColorAt(1.0, borde);
        p.setPen(Qt::NoPen);
        p.setBrush(degradado);
        p.drawEllipse(QRectF(0, 0, lado, lado));
    }
    return px;
}

QPixmap pintarSombra() {
    QPixmap px(52, 16);
    px.fill(Qt::transparent);
    {
        QPainter p(&px);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 38));
        p.drawEllipse(QRectF(0, 0, 52, 16));
        p.setBrush(QColor(0, 0, 0, 48));
        p.drawEllipse(QRectF(5, 2, 42, 12));
        p.setBrush(QColor(0, 0, 0, 60));
        p.drawEllipse(QRectF(11, 4, 30, 8));
    }
    return px;
}

QPixmap armarPolvorin(const QPixmap &escenario) {
    QPixmap lienzo(170, 130);
    lienzo.fill(Qt::transparent);
    {
        QPainter p(&lienzo);
        p.drawPixmap(2, 45, recorte(escenario, Atlas::BARRIL_ROJO));
        p.drawPixmap(108, 45, recorte(escenario, Atlas::BARRIL_ROJO));
        p.drawPixmap(30, 34, recorte(escenario, Atlas::CAJON_BALAS));
        p.drawPixmap(62, 70, recorte(escenario, Atlas::BARRIL_ROJO).scaledToHeight(56, Qt::SmoothTransformation));
    }
    return lienzo;
}

void cortarCeldas(const QPixmap &hoja, int ancho, int alto, int columnas, int filas, float escala,
                  QVector<QPixmap> &normal, QVector<QPixmap> &espejo) {
    normal.clear();
    espejo.clear();
    if (hoja.isNull())
        return;
    const int anchoPx = qMax(1, qRound(ancho * escala));
    const int altoPx = qMax(1, qRound(alto * escala));
    normal.reserve(columnas * filas);
    espejo.reserve(columnas * filas);
    for (int fila = 0; fila < filas; ++fila) {
        for (int col = 0; col < columnas; ++col) {
            const QPixmap celda = hoja.copy(col * ancho, fila * alto, ancho, alto)
                                      .scaled(anchoPx, altoPx, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            normal.push_back(celda);
            espejo.push_back(espejar(celda));
        }
    }
}

QPolygonF sectorCono(float radio, float apertura) {
    QPolygonF cono;
    cono << QPointF(0, 0);
    const int tramos = 18;
    for (int i = 0; i <= tramos; ++i) {
        const float a = -apertura * 0.5f + apertura * float(i) / float(tramos);
        cono << QPointF(std::cos(a) * radio, std::sin(a) * radio);
    }
    return cono;
}

QPolygonF puntaFlecha() {
    QPolygonF flecha;
    flecha << QPointF(-11, -18) << QPointF(11, -18) << QPointF(0, 0);
    return flecha;
}

QString formatoTiempo(int segundos) {
    return QStringLiteral("%1:%2").arg(segundos / 60).arg(segundos % 60, 2, 10, QChar('0'));
}
}

EscenaAsalto::EscenaAsalto() {
    setItemIndexMethod(QGraphicsScene::NoIndex);
    setSceneRect(0, 0, 1600, 900);
    setBackgroundBrush(QColor(40, 34, 28));
    efectos.reserve(kMaxEfectos);
    reserva.reserve(kMaxEfectos);
    eventos.reserve(32);
    juego = new Juego();
    juego->iniciarNivel(NIVEL_ASALTO, DIF_NORMAL);
    nivel = dynamic_cast<NivelAsalto *>(juego->nivelActivo());
    if (nivel)
        setSceneRect(0, 0, nivel->getAnchoMundo(), nivel->getAltoMundo());
    cargarSprites();
    armarEscenario();
    armarConos();
    armarHud();
    if (nivel && nivel->jugador()) {
        const float ancho = nivel->getAnchoMundo();
        const float alto = nivel->getAltoMundo();
        camX = limitar(nivel->jugador()->getCentroX() - vistaAncho * 0.45f, 0.f, std::max(0.f, ancho - vistaAncho));
        camY = limitar(nivel->jugador()->getCentroY() - vistaAlto * 0.55f, 0.f, std::max(0.f, alto - vistaAlto));
    }
}

EscenaAsalto::~EscenaAsalto() {
    delete juego;
}

void EscenaAsalto::cargarSprites() {
    const QPixmap hojaRocket(QStringLiteral(":/nivel2/rocket.png"));
    const QPixmap hojaSoldado(QStringLiteral(":/nivel2/soldados.png"));
    const QPixmap escenario(QStringLiteral(":/nivel2/escenario.png"));

    cortarCeldas(hojaRocket, Atlas::ROCKET_ANCHO_CELDA, Atlas::ROCKET_ALTO_CELDA, Atlas::ROCKET_COLUMNAS,
                 Atlas::ROCKET_FILAS, Atlas::ESCALA_ROCKET, rocketNormal, rocketEspejo);
    cortarCeldas(hojaSoldado, Atlas::SOLDADO_ANCHO_CELDA, Atlas::SOLDADO_ALTO_CELDA, Atlas::SOLDADO_COLUMNAS,
                 Atlas::SOLDADO_FILAS, Atlas::ESCALA_SOLDADO, soldadoNormal, soldadoEspejo);

    aspectoRocket = bloque(48, 64, QColor(70, 98, 170));
    aspectoSoldado = bloque(40, 60, QColor(176, 58, 52));
    rocketAncho = 48.f;
    rocketPieX = 24.f;
    rocketPieY = 64.f;
    soldadoAncho = 40.f;
    soldadoPieX = 20.f;
    soldadoPieY = 60.f;
    if (!rocketNormal.isEmpty()) {
        rocketAncho = float(qRound(Atlas::ROCKET_ANCHO_CELDA * Atlas::ESCALA_ROCKET));
        rocketPieX = Atlas::ROCKET_PIE_X * Atlas::ESCALA_ROCKET;
        rocketPieY = Atlas::ROCKET_PIE_Y * Atlas::ESCALA_ROCKET;
    }
    if (!soldadoNormal.isEmpty()) {
        soldadoAncho = float(qRound(Atlas::SOLDADO_ANCHO_CELDA * Atlas::ESCALA_SOLDADO));
        soldadoPieX = Atlas::SOLDADO_PIE_X * Atlas::ESCALA_SOLDADO;
        soldadoPieY = Atlas::SOLDADO_PIE_Y * Atlas::ESCALA_SOLDADO;
    }

    if (escenario.isNull()) {
        torreA = bloque(60, 110, QColor(120, 92, 54));
        torreB = torreA;
        polvorin = bloque(120, 96, QColor(150, 104, 54));
        polvorinRoto = bloque(120, 96, QColor(70, 54, 40));
        bandera = bloque(30, 90, QColor(190, 52, 46));
        banderaCaida = bloque(90, 30, QColor(110, 70, 60));
        canon = bloque(100, 60, QColor(52, 56, 62));
        canonRoto = bloque(100, 60, QColor(36, 34, 32));
        barril = bloque(34, 50, QColor(176, 58, 42));
        barrera = bloque(int(kBarreraAncho), 48, QColor(170, 142, 92));
    } else {
        torreA = escalar(recorte(escenario, Atlas::TORRE_A), 0.55f);
        torreB = escalar(recorte(escenario, Atlas::TORRE_B), 0.55f);
        polvorin = escalar(armarPolvorin(escenario), 0.75f);
        polvorinRoto = oscurecer(polvorin, QColor(28, 22, 18, 200));
        bandera = escalar(recorte(escenario, Atlas::BANDERA_REAL_ROJA), 0.85f);
        banderaCaida = oscurecer(bandera, QColor(30, 20, 20, 90))
                           .transformed(QTransform().rotate(84), Qt::SmoothTransformation);
        canon = escalar(recorte(escenario, Atlas::CANON_2), 0.75f);
        canonRoto = oscurecer(canon, QColor(20, 18, 16, 200))
                        .transformed(QTransform().rotate(-9), Qt::SmoothTransformation);
        humoDestruccion = escalar(recorte(escenario, Atlas::HUMO_2), 0.8f);
        explosion1 = recorte(escenario, Atlas::EXPLOSION_1);
        explosion2 = recorte(escenario, Atlas::EXPLOSION_2_FUEGO);
        explosion3 = recorte(escenario, Atlas::EXPLOSION_3);
        humoSubida = recorte(escenario, Atlas::HUMO_1);
        barril = escalar(recorte(escenario, Atlas::BARRIL_ROJO), 0.62f);
        barrera = escalar(recorte(escenario, Atlas::SACOS_2), kBarreraAncho / float(Atlas::SACOS_2.width()));
    }
    canonEspejo = espejar(canon);
    canonRotoEspejo = espejar(canonRoto);

    sombraSuave = pintarSombra();
    chispa = esfera(24, QColor(255, 250, 200, 255), QColor(255, 150, 40, 0));
    balaRocket = esfera(10, QColor(255, 246, 170), QColor(238, 170, 40));
    balaEnemiga = esfera(10, QColor(255, 150, 120), QColor(170, 36, 30));
    bolaCanon = esfera(18, QColor(104, 106, 112), QColor(22, 22, 26));
    granadaVuelo = esfera(16, QColor(132, 180, 90), QColor(38, 66, 30));

    const QPixmap hoja(QStringLiteral(":/nivel1/sprint_vidas_municion.png"));
    if (!hoja.isNull())
        prepararMarcador(hoja.toImage().convertToFormat(QImage::Format_ARGB32), piezasHud);
}

void EscenaAsalto::armarEscenario() {
    const float ancho = nivel ? nivel->getAnchoMundo() : 1600.f;
    const float alto = nivel ? nivel->getAltoMundo() : 900.f;

    const QPixmap fondo(QStringLiteral(":/nivel2/fondo.png"));
    if (!fondo.isNull()) {
        auto *imagen = addPixmap(fondo.scaled(qRound(ancho), qRound(alto), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        imagen->setZValue(0);
    } else {
        auto *suelo = addRect(0, 0, ancho, alto, Qt::NoPen, QColor(104, 82, 54));
        suelo->setZValue(0);
    }

    if (!nivel)
        return;

    Estructura *lista[3] = {nivel->getPolvorin(), nivel->getBandera(), nivel->getCanon()};
    for (Estructura *objetivo : lista) {
        auto *flecha = addPolygon(puntaFlecha(), QPen(QColor(50, 28, 10), 2), QBrush(QColor(255, 208, 70)));
        flecha->setZValue(kZFlecha);
        flechas.push_back(flecha);
        objetivos.push_back(objetivo);
    }
}

void EscenaAsalto::armarConos() {
    if (!nivel)
        return;
    const std::vector<Estructura *> &torres = nivel->getTorres();
    float mayor = 1.f;
    for (Estructura *torre : torres)
        mayor = std::max(mayor, torre->getRadioCono());

    QRadialGradient normal(0, 0, mayor);
    normal.setColorAt(0.0, QColor(255, 226, 120, 96));
    normal.setColorAt(1.0, QColor(255, 226, 120, 14));
    brochaCono[0] = QBrush(normal);
    lapizCono[0] = QPen(QColor(255, 226, 120, 120), 1.5);
    QRadialGradient alerta(0, 0, mayor);
    alerta.setColorAt(0.0, QColor(255, 70, 50, 130));
    alerta.setColorAt(1.0, QColor(255, 70, 50, 34));
    brochaCono[1] = QBrush(alerta);
    lapizCono[1] = QPen(QColor(255, 90, 70, 150), 1.5);

    for (Estructura *torre : torres) {
        const float radio = torre->getRadioCono();
        auto *cono = addPolygon(sectorCono(radio, torre->getApertura()), lapizCono[0], brochaCono[0]);
        cono->setPos(torre->getCentroX(), torre->getCentroY());
        cono->setZValue(kZCono);
        conos.push_back(cono);
        torresVigiladas.push_back(torre);
        conoTono.push_back(0);
    }
}

QGraphicsSimpleTextItem *EscenaAsalto::crearTexto(const QString &texto, int tamano, const QColor &color, QGraphicsItem *padre) {
    auto *item = addSimpleText(texto);
    QFont letra(QStringLiteral("Georgia"));
    letra.setPixelSize(tamano);
    letra.setBold(true);
    item->setFont(letra);
    item->setBrush(color);
    if (padre) {
        item->setParentItem(padre);
        item->setZValue(1);
    }
    return item;
}

QGraphicsPathItem *EscenaAsalto::crearPildora(float ancho, float alto, const QColor &color, QGraphicsItem *padre) {
    QPainterPath camino;
    const float radio = std::min(alto * 0.5f, 10.f);
    camino.addRoundedRect(QRectF(0, 0, ancho, alto), radio, radio);
    auto *item = addPath(camino, QPen(Qt::NoPen), QBrush(color));
    if (padre) {
        item->setParentItem(padre);
        item->setZValue(1);
    }
    return item;
}

QGraphicsPathItem *EscenaAsalto::crearAviso(const QString &texto, const QColor &color) {
    auto *etiqueta = crearTexto(texto, 26, QColor(255, 255, 255), nullptr);
    const float ancho = etiqueta->boundingRect().width() + 48.f;
    auto *aviso = crearPildora(ancho, 44, color, nullptr);
    aviso->setZValue(kZHud);
    etiqueta->setParentItem(aviso);
    etiqueta->setZValue(1);
    etiqueta->setPos(24, (44.f - etiqueta->boundingRect().height()) * 0.5f);
    aviso->hide();
    return aviso;
}

void EscenaAsalto::armarHud() {
    panel = addRect(0, 0, kPanelAncho, 98, QPen(Qt::NoPen), QBrush(QColor(18, 12, 8, 150)));
    panel->setZValue(kZHud - 1.f);

    hudEtiquetaTiempo = crearTexto(QStringLiteral("Tiempo"), 13, QColor(255, 255, 255), panel);
    hudEtiquetaTiempo->setPos(12, 15);
    hudReloj = crearTexto(QStringLiteral("0:00"), 22, QColor(255, 255, 255), panel);
    hudReloj->setPos(kPanelAncho - 12.f - hudReloj->boundingRect().width(), 8);

    static const char *nombres[3] = {"Polvorín", "Bandera", "Cañón"};
    for (int i = 0; i < 3; ++i) {
        const float cx = 12.f + float(i) * 92.f;
        hudPunto[i] = addEllipse(-6, -6, 12, 12, QPen(QColor(200, 200, 200), 2), QBrush(Qt::NoBrush));
        hudPunto[i]->setParentItem(panel);
        hudPunto[i]->setZValue(1);
        hudPunto[i]->setPos(cx + 6.f, 50.f);
        hudTarea[i] = crearTexto(QString::fromUtf8(nombres[i]), 13, QColor(230, 230, 230), panel);
        hudTarea[i]->setPos(cx + 16.f, 41.f);
    }

    auto *etiquetaAlarma = crearTexto(QStringLiteral("Alarma"), 13, QColor(255, 255, 255), panel);
    etiquetaAlarma->setPos(12, 68);
    for (int i = 0; i < 3; ++i) {
        hudAlarma[i] = addEllipse(-8, -8, 16, 16, QPen(QColor(0, 0, 0, 170), 1), QBrush(QColor(90, 90, 90, 210)));
        hudAlarma[i]->setParentItem(panel);
        hudAlarma[i]->setZValue(1);
        hudAlarma[i]->setPos(92.f + float(i) * 22.f, 77.f);
    }

    avisoRefuerzos = crearAviso(QStringLiteral("¡REFUERZOS DEL CORONEL!"), QColor(176, 40, 36, 215));
    avisoResistencia = crearAviso(QStringLiteral("¡RESISTE! LOS PATRIOTAS YA VIENEN"), QColor(196, 112, 24, 225));
}

void EscenaAsalto::pulsar(int tecla) {
    teclas.insert(tecla);
    if (!nivel || nivel->estaTerminado() || !nivel->jugador())
        return;
    if (tecla == Qt::Key_Shift)
        nivel->jugadorDash();
    if (tecla == Qt::Key_F)
        nivel->jugadorActivaFuria();
    if (tecla == Qt::Key_G) {
        Rocket *rocket = nivel->jugador();
        const float ax = hayApunte ? punteroX : rocket->getCentroX() + 180.f;
        const float ay = hayApunte ? punteroY : rocket->getCentroY();
        nivel->jugadorLanzaGranada(ax, ay);
    }
}

void EscenaAsalto::soltar(int tecla) {
    teclas.remove(tecla);
}

void EscenaAsalto::apuntar(float x, float y) {
    punteroX = x;
    punteroY = y;
    hayApunte = true;
}

void EscenaAsalto::avanzar(float dt) {
    if (!nivel)
        return;
    tiempo += dt;
    rocketDisparoT = std::max(0.f, rocketDisparoT - dt);
    rocketLanzaT = std::max(0.f, rocketLanzaT - dt);
    rocketDanioT = std::max(0.f, rocketDanioT - dt);

    Rocket *rocket = nivel->jugador();
    if (rocket && !nivel->estaTerminado()) {
        if (teclas.contains(Qt::Key_A) || teclas.contains(Qt::Key_Left))
            nivel->jugadorMueve('L');
        if (teclas.contains(Qt::Key_D) || teclas.contains(Qt::Key_Right))
            nivel->jugadorMueve('R');
        if (teclas.contains(Qt::Key_W) || teclas.contains(Qt::Key_Up))
            nivel->jugadorMueve('U');
        if (teclas.contains(Qt::Key_S) || teclas.contains(Qt::Key_Down))
            nivel->jugadorMueve('D');
        if (teclas.contains(Qt::Key_Space)) {
            const float ax = hayApunte ? punteroX : rocket->getCentroX() + (rocket->getDireccion() == 'L' ? -360.f : 360.f);
            const float ay = hayApunte ? punteroY : rocket->getCentroY();
            nivel->jugadorDispara(ax, ay);
        }
    }

    juego->actualizar(dt);
    nivel->tomarEventos(eventos);
    if (sonido)
        sonido->procesar(eventos);
    eventos.clear();

    rocket = nivel->jugador();
    if (rocket) {
        if (sonido)
            sonido->ritmo(QStringLiteral("paso"), rocket->getVidas() > 0 && rocket->estadoVisual() == 1, 300);
        if (rocket->getVidas() <= 0)
            rocketMuerteT += dt;
        const float ancho = nivel->getAnchoMundo();
        const float alto = nivel->getAltoMundo();
        const float deseoX = limitar(rocket->getCentroX() - vistaAncho * 0.45f, 0.f, std::max(0.f, ancho - vistaAncho));
        const float deseoY = limitar(rocket->getCentroY() - vistaAlto * 0.55f, 0.f, std::max(0.f, alto - vistaAlto));
        const float paso = std::min(1.f, dt * 5.f);
        camX += (deseoX - camX) * paso;
        camY += (deseoY - camY) * paso;
    }

    sincronizar(dt);
    actualizarEfectos(dt);
    actualizarConos();
    actualizarObjetivos();
    actualizarHud();
    dibujarMarcador();
}

void EscenaAsalto::fijarProporcion(float aspecto) {
    if (!nivel || aspecto < 0.5f || aspecto > 6.f)
        return;
    const float ancho = nivel->getAnchoMundo();
    const float alto = nivel->getAltoMundo();
    float w = kVistaAncho;
    float h = kVistaAlto;
    if (aspecto >= kVistaAncho / kVistaAlto) {
        w = std::min(kVistaAlto * aspecto, ancho);
        h = w / aspecto;
    } else {
        h = std::min(kVistaAncho / aspecto, alto);
        w = h * aspecto;
    }
    vistaAncho = w;
    vistaAlto = h;
    camX = limitar(camX, 0.f, std::max(0.f, ancho - vistaAncho));
    camY = limitar(camY, 0.f, std::max(0.f, alto - vistaAlto));
}

QRectF EscenaAsalto::rectCamara() const {
    return QRectF(camX, camY, vistaAncho, vistaAlto);
}

QString EscenaAsalto::textoHud() const {
    return {};
}

QString EscenaAsalto::mensajeFinal() const {
    if (!nivel || !nivel->estaTerminado())
        return {};
    if (nivel->fueGanado())
        return QStringLiteral("Misión cumplida\nEl campamento español quedó saboteado.");
    if (nivel->getTiempoRestante() <= 0.f)
        return QStringLiteral("Misión fallida\nSe acabó el tiempo del asalto.");
    return QStringLiteral("Misión fallida\nTe derrotaron en el campamento.");
}

bool EscenaAsalto::gano() const {
    return nivel && nivel->fueGanado();
}

bool EscenaAsalto::acabo() const {
    return nivel && nivel->estaTerminado();
}

const QPixmap &EscenaAsalto::celdaRocket(int fila, int col, bool espejo) const {
    const QVector<QPixmap> &lista = espejo ? rocketEspejo : rocketNormal;
    const int indice = fila * Atlas::ROCKET_COLUMNAS + col;
    if (indice < 0 || indice >= lista.size())
        return aspectoRocket;
    return lista.at(indice);
}

const QPixmap &EscenaAsalto::celdaSoldado(int fila, int col, bool espejo) const {
    const QVector<QPixmap> &lista = espejo ? soldadoEspejo : soldadoNormal;
    const int indice = fila * Atlas::SOLDADO_COLUMNAS + col;
    if (indice < 0 || indice >= lista.size())
        return aspectoSoldado;
    return lista.at(indice);
}

void EscenaAsalto::poner(Pieza &pieza, const QPixmap &imagen, float x, float y) {
    const qint64 llave = imagen.cacheKey();
    if (llave != pieza.llave) {
        pieza.item->setPixmap(imagen);
        pieza.llave = llave;
    }
    pieza.item->setPos(x, y);
}

QGraphicsPixmapItem *EscenaAsalto::tomarItem() {
    if (!reserva.empty()) {
        QGraphicsPixmapItem *item = reserva.back();
        reserva.pop_back();
        return item;
    }
    QGraphicsPixmapItem *item = addPixmap(QPixmap());
    item->setTransformationMode(Qt::SmoothTransformation);
    return item;
}

void EscenaAsalto::devolverItem(QGraphicsPixmapItem *item) {
    item->hide();
    reserva.push_back(item);
}

void EscenaAsalto::crearPieza(Entidad *entidad, const std::vector<Entidad *> &lista) {
    Pieza pieza;
    pieza.clase = entidad->clase();
    pieza.item = addPixmap(QPixmap());
    pieza.x = entidad->getCentroX();
    pieza.pieY = entidad->getY() + entidad->getAlto();

    if (pieza.clase == CLASE_ROCKET || pieza.clase == CLASE_ENEMIGO) {
        pieza.sombra = addPixmap(sombraSuave);
        pieza.sombra->setZValue(kZSombra);
    }
    if (pieza.clase == CLASE_ROCKET) {
        pieza.anillo = addEllipse(-48, -20, 96, 40, QPen(QColor(255, 90, 60, 170), 2), QBrush(QColor(255, 70, 40, 60)));
        pieza.anillo->setZValue(kZSombra + 0.1f);
        pieza.anillo->hide();
    }

    if (pieza.clase == CLASE_PROYECTIL) {
        auto *bala = static_cast<Proyectil *>(entidad);
        pieza.bombeado = bala->esBombeado();
        pieza.deRocket = bala->esDeRocket();
        pieza.destinoX = bala->getDestinoX();
        pieza.destinoY = bala->getDestinoY();
        pieza.radio = bala->getRadioExplosion();
        pieza.item->setZValue(kZProyectil);

        if (pieza.bombeado) {
            const float r = std::max(10.f, pieza.radio);
            const QColor color = pieza.deRocket ? QColor(240, 230, 140) : QColor(236, 74, 50);
            pieza.anillo = addEllipse(-r, -r, r * 2.f, r * 2.f, QPen(QColor(color.red(), color.green(), color.blue(), 190), 2),
                                      QBrush(QColor(color.red(), color.green(), color.blue(), 38)));
            pieza.anillo->setPos(pieza.destinoX, pieza.destinoY);
            pieza.anillo->setZValue(kZAviso);
        }

        if (pieza.deRocket && !pieza.bombeado) {
            rocketDisparoT = kDisparoRocket;
        } else if (pieza.deRocket) {
            rocketLanzaT = kLanzaRocket;
        } else if (!pieza.bombeado) {
            Entidad *cercano = nullptr;
            float mejor = 1.0e12f;
            for (Entidad *otro : lista) {
                if (otro->clase() != CLASE_ENEMIGO)
                    continue;
                const float dx = otro->getCentroX() - entidad->getCentroX();
                const float dy = otro->getCentroY() - entidad->getCentroY();
                const float d2 = dx * dx + dy * dy;
                if (d2 < mejor) {
                    mejor = d2;
                    cercano = otro;
                }
            }
            auto it = piezas.find(cercano);
            if (cercano && it != piezas.end()) {
                it.value().disparoT = kDisparoSoldado;
                it.value().variante ^= 1;
            }
        } else if (nivel && nivel->getCanon() && nivel->jugador()) {
            Estructura *boca = nivel->getCanon();
            const float lado = nivel->jugador()->getCentroX() > boca->getCentroX() ? 1.f : -1.f;
            lanzarEfecto(EF_EXPLOSION, boca->getCentroX() + lado * 42.f, boca->getCentroY() - 38.f, 24.f);
        }
    }

    if (pieza.clase == CLASE_ESTRUCTURA && nivel) {
        const std::vector<Estructura *> &torres = nivel->getTorres();
        pieza.variante = (!torres.empty() && torres.front() == entidad) ? 0 : 1;
    }

    piezas.insert(entidad, pieza);
}

void EscenaAsalto::sincronizar(float dt) {
    ++cuadroActual;
    explosionesCuadro = 0;
    const std::vector<Entidad *> &lista = nivel->obtenerEntidades();
    for (Entidad *entidad : lista) {
        auto it = piezas.find(entidad);
        if (!entidad->estaViva() && entidad->clase() != CLASE_ROCKET) {
            if (it != piezas.end()) {
                retirarPieza(it.value());
                piezas.erase(it);
            }
            continue;
        }
        if (it != piezas.end() && it.value().clase != entidad->clase()) {
            retirarPieza(it.value());
            piezas.erase(it);
            it = piezas.end();
        }
        if (it == piezas.end()) {
            crearPieza(entidad, lista);
            it = piezas.find(entidad);
            if (it == piezas.end())
                continue;
        }
        Pieza &pieza = it.value();
        pieza.cuadro = cuadroActual;
        if (pieza.disparoT > 0.f)
            pieza.disparoT -= dt;

        switch (pieza.clase) {
        case CLASE_ROCKET:
            vestirRocket(static_cast<Rocket *>(entidad), pieza);
            break;
        case CLASE_ENEMIGO:
            vestirSoldado(static_cast<Enemigo *>(entidad), pieza);
            break;
        case CLASE_ESTRUCTURA:
            vestirEstructura(static_cast<Estructura *>(entidad), pieza);
            break;
        case CLASE_PROYECTIL:
            vestirProyectil(entidad, pieza);
            break;
        default:
            break;
        }
    }

    for (auto it = piezas.begin(); it != piezas.end();) {
        if (it.value().cuadro == cuadroActual) {
            ++it;
            continue;
        }
        retirarPieza(it.value());
        it = piezas.erase(it);
    }
}

void EscenaAsalto::vestirRocket(Rocket *rocket, Pieza &pieza) {
    const float cx = rocket->getCentroX();
    const float pie = rocket->getY() + rocket->getAlto();
    const char dir = rocket->getDireccion();

    if (rocketVidasPrev >= 0 && (rocket->getVidas() < rocketVidasPrev || rocket->getSalud() < rocketSaludPrev - 0.01f))
        rocketDanioT = kDanioRocket;
    rocketVidasPrev = rocket->getVidas();
    rocketSaludPrev = rocket->getSalud();

    if (dir == 'R')
        rocketMiraDer = true;
    else if (dir == 'L')
        rocketMiraDer = false;
    if ((rocketDisparoT > 0.f || rocketLanzaT > 0.f) && hayApunte)
        rocketMiraDer = punteroX >= cx;

    int fila = Atlas::ROCKET_REPOSO;
    int col = int(tiempo * 4.f) % Atlas::ROCKET_COLUMNAS;
    if (rocket->getVidas() <= 0) {
        fila = Atlas::ROCKET_DANIO;
        col = rocketMuerteT < 0.35f ? 4 : 5;
    } else if (rocket->estadoVisual() == 3) {
        fila = Atlas::ROCKET_DASH;
        col = int(tiempo * 16.f) % 4;
    } else if (rocketDanioT > 0.f) {
        fila = Atlas::ROCKET_DANIO;
        col = 1 + int(tiempo * 12.f) % 2;
    } else if (rocketLanzaT > 0.f) {
        fila = Atlas::ROCKET_DISPARA_Y_GRANADA;
        col = 3 + std::min(2, int((kLanzaRocket - rocketLanzaT) / (kLanzaRocket / 3.f)));
    } else if (rocketDisparoT > 0.f) {
        fila = Atlas::ROCKET_DISPARA;
        col = 2 + int(tiempo * 18.f) % 3;
    } else if (rocket->estadoVisual() == 1) {
        fila = (dir == 'U' || dir == 'D') ? Atlas::ROCKET_CAMINA_1 : Atlas::ROCKET_CAMINA_DER;
        col = int(tiempo * 10.f) % Atlas::ROCKET_COLUMNAS;
    }

    const bool espejo = !rocketMiraDer;
    const QPixmap &imagen = celdaRocket(fila, col, espejo);
    const float pieX = espejo ? rocketAncho - rocketPieX : rocketPieX;
    poner(pieza, imagen, cx - pieX, pie - rocketPieY);
    pieza.item->setZValue(kZPersonaje + pie * 0.001f);
    pieza.item->setOpacity(!rocket->esInvulnerable() || std::fmod(tiempo, 0.24f) < 0.12f ? 1.0 : 0.35);
    pieza.x = cx;
    pieza.pieY = pie;
    pieza.espejo = espejo;

    if (pieza.sombra)
        pieza.sombra->setPos(cx - 26.f, pie - 9.f);

    if (pieza.anillo) {
        const bool furioso = rocket->estaFurioso() && rocket->getVidas() > 0;
        pieza.anillo->setVisible(furioso);
        if (furioso) {
            pieza.anillo->setPos(cx, pie - 6.f);
            pieza.anillo->setOpacity(0.65 + 0.35 * std::sin(tiempo * 9.f));
        }
    }
}

void EscenaAsalto::vestirSoldado(Enemigo *enemigo, Pieza &pieza) {
    Rocket *rocket = nivel->jugador();
    const float cx = enemigo->getCentroX();
    const float pie = enemigo->getY() + enemigo->getAlto();
    const bool seMueve = std::fabs(cx - pieza.x) + std::fabs(pie - pieza.pieY) > 0.15f;
    bool derecha = enemigo->getDireccion() != 'L';

    int fila = Atlas::SOLDADO_POSES;
    int col = int(tiempo * 2.f) % 3;
    if (pieza.disparoT > 0.f) {
        if (rocket)
            derecha = rocket->getCentroX() >= cx;
        fila = pieza.variante ? Atlas::SOLDADO_DISPARA_1 : Atlas::SOLDADO_DISPARA_0;
        col = std::max(0, std::min(5, int((kDisparoSoldado - pieza.disparoT) * 20.f)));
    } else if (seMueve || enemigo->estadoVisual() != 0) {
        fila = enemigo->estadoVisual() == 0 ? Atlas::SOLDADO_CAMINA_2 : Atlas::SOLDADO_CAMINA_3;
        col = int(tiempo * (enemigo->estadoVisual() == 0 ? 8.f : 12.f)) % Atlas::SOLDADO_COLUMNAS;
    }

    const bool espejo = !derecha;
    const QPixmap &imagen = celdaSoldado(fila, col, espejo);
    const float pieX = espejo ? soldadoAncho - soldadoPieX : soldadoPieX;
    poner(pieza, imagen, cx - pieX, pie - soldadoPieY);
    pieza.item->setZValue(kZPersonaje + pie * 0.001f);
    pieza.x = cx;
    pieza.pieY = pie;
    pieza.espejo = espejo;

    if (pieza.sombra)
        pieza.sombra->setPos(cx - 26.f, pie - 9.f);
}

void EscenaAsalto::vestirEstructura(Estructura *estructura, Pieza &pieza) {
    const bool rota = estructura->estaDestruida();
    const short tipo = estructura->getTipo();
    const float cx = estructura->getCentroX();
    const float base = estructura->getY() + estructura->getAlto();
    const QPixmap *imagen = nullptr;
    float desfase = 0.f;

    switch (tipo) {
    case EST_TORRE:
        imagen = pieza.variante == 0 ? &torreA : &torreB;
        break;
    case EST_POLVORIN:
        imagen = rota ? &polvorinRoto : &polvorin;
        break;
    case EST_BANDERA:
        imagen = rota ? &banderaCaida : &bandera;
        break;
    case EST_BARRIL:
        imagen = &barril;
        break;
    case EST_PLATAFORMA:
        imagen = &barrera;
        break;
    case EST_CANON: {
        Rocket *rocket = nivel->jugador();
        const bool espejo = rocket && rocket->getCentroX() > cx;
        pieza.espejo = espejo;
        if (rota)
            imagen = espejo ? &canonRotoEspejo : &canonRoto;
        else
            imagen = espejo ? &canonEspejo : &canon;
        desfase = 6.f;
        break;
    }
    default:
        return;
    }

    poner(pieza, *imagen, cx - imagen->width() / 2.f, base + desfase - imagen->height());
    pieza.item->setZValue(kZPersonaje + base * 0.001f);
    if (tipo == EST_BARRIL)
        pieza.item->setVisible(!rota);

    if (rota && pieza.estado == 0) {
        pieza.estado = 1;
        float radio = 40.f;
        float retraso = 0.f;
        if (tipo == EST_POLVORIN)
            radio = 95.f;
        else if (tipo == EST_CANON)
            radio = 60.f;
        else if (tipo == EST_BARRIL) {
            radio = 85.f;
            retraso = 0.07f * float(explosionesCuadro++);
        }
        lanzarEfecto(EF_EXPLOSION, cx, estructura->getCentroY(), radio, false, retraso);
        const bool conHumo = tipo == EST_POLVORIN || tipo == EST_CANON;
        if (conHumo && !humoDestruccion.isNull()) {
            pieza.humo = addPixmap(humoDestruccion);
            pieza.humo->setZValue(kZHumo);
            pieza.humo->setOpacity(0.55);
        }
    }

    if (pieza.humo) {
        const float oscila = std::sin(tiempo * 1.7f + cx);
        pieza.humo->setPos(cx - pieza.humo->pixmap().width() / 2.f + oscila * 5.f,
                           base - imagen->height() * 0.75f - pieza.humo->pixmap().height() * 0.5f - 6.f * oscila);
        pieza.humo->setOpacity(0.48 + 0.12 * oscila);
    }
}

void EscenaAsalto::vestirProyectil(Entidad *entidad, Pieza &pieza) {
    const QPixmap &imagen = pieza.bombeado ? (pieza.deRocket ? granadaVuelo : bolaCanon)
                                           : (pieza.deRocket ? balaRocket : balaEnemiga);
    pieza.x = entidad->getCentroX();
    pieza.pieY = entidad->getCentroY();
    poner(pieza, imagen, pieza.x - imagen.width() / 2.f, pieza.pieY - imagen.height() / 2.f);
}

void EscenaAsalto::retirarPieza(Pieza &pieza) {
    if (pieza.clase == CLASE_ENEMIGO)
        lanzarEfecto(EF_CAIDA, pieza.x, pieza.pieY, 0.f, pieza.espejo);
    else if (pieza.clase == CLASE_PROYECTIL && pieza.bombeado)
        lanzarEfecto(EF_EXPLOSION, pieza.destinoX, pieza.destinoY, pieza.radio);
    QGraphicsItem *items[4] = {pieza.item, pieza.sombra, pieza.humo, pieza.anillo};
    for (QGraphicsItem *item : items) {
        if (!item)
            continue;
        removeItem(item);
        delete item;
    }
    pieza.item = nullptr;
    pieza.sombra = nullptr;
    pieza.humo = nullptr;
    pieza.anillo = nullptr;
}

void EscenaAsalto::lanzarEfecto(short tipo, float x, float y, float radio, bool espejo, float retraso) {
    if (int(efectos.size()) >= kMaxEfectos)
        return;
    Efecto efecto;
    efecto.tipo = tipo;
    efecto.edad = -retraso;
    efecto.x = x;
    efecto.y = y;
    efecto.radio = radio;
    efecto.espejo = espejo;
    efecto.item = tomarItem();
    efecto.item->setZValue(kZEfecto);
    efecto.item->hide();
    efectos.push_back(efecto);
}

void EscenaAsalto::actualizarEfectos(float dt) {
    for (int i = int(efectos.size()) - 1; i >= 0; --i) {
        Efecto &efecto = efectos[i];
        efecto.edad += dt;
        const float t = efecto.edad;

        const float duracion = efecto.tipo == EF_CAIDA ? kCaidaDuracion : kExplosionDuracion;

        if (t >= duracion) {
            devolverItem(efecto.item);
            efectos.erase(efectos.begin() + i);
            continue;
        }
        if (t < 0.f)
            continue;

        const QPixmap *imagen = &chispa;
        float escala = 1.f;
        float opacidad = 1.f;
        float subida = 0.f;

        if (efecto.tipo == EF_EXPLOSION) {
            const float base = efecto.radio * 2.3f / 153.f;
            if (t < 0.15f) {
                imagen = &explosion1;
                escala = base * (0.6f + 0.4f * t / 0.15f);
            } else if (t < 0.35f) {
                imagen = &explosion3;
                escala = base;
            } else if (t < 0.65f) {
                imagen = &explosion2;
                escala = base;
                opacidad = 1.f - 0.3f * (t - 0.35f) / 0.3f;
            } else {
                imagen = &humoSubida;
                escala = base * 0.9f;
                opacidad = 0.7f * (1.f - (t - 0.65f) / 0.75f);
                subida = 40.f * (t - 0.65f);
            }
            if (imagen->isNull())
                imagen = &chispa;
        } else {
            int fila = Atlas::SOLDADO_CAIDO;
            int col = 5;
            if (t < 0.12f) {
                fila = Atlas::SOLDADO_CAE;
                col = 3;
            } else if (t < 0.24f) {
                fila = Atlas::SOLDADO_CAE;
                col = 4;
            } else if (t < 0.36f) {
                fila = Atlas::SOLDADO_CAE;
                col = 5;
            }
            imagen = &celdaSoldado(fila, col, efecto.espejo);
            opacidad = t < 1.3f ? 1.f : std::max(0.f, 1.f - (t - 1.3f) / 0.6f);
        }

        QGraphicsPixmapItem *item = efecto.item;
        if (!item->isVisible())
            item->show();
        if (item->pixmap().cacheKey() != imagen->cacheKey())
            item->setPixmap(*imagen);
        item->setOpacity(std::max(0.f, opacidad));
        if (efecto.tipo == EF_CAIDA) {
            item->setOffset(0, 0);
            item->setScale(1.0);
            item->setPos(efecto.x - (efecto.espejo ? soldadoAncho - soldadoPieX : soldadoPieX), efecto.y - soldadoPieY);
            item->setZValue(kZCuerpo);
        } else {
            item->setOffset(-imagen->width() / 2.0, -imagen->height() / 2.0);
            item->setScale(escala);
            item->setPos(efecto.x, efecto.y - subida);
            item->setZValue(kZEfecto);
        }
    }
}

void EscenaAsalto::actualizarConos() {
    Rocket *rocket = nivel->jugador();
    for (int i = 0; i < conos.size(); ++i) {
        Estructura *torre = torresVigiladas.at(i);
        QGraphicsPolygonItem *cono = conos.at(i);
        const bool activa = !torre->estaDestruida();
        if (activa != cono->isVisible())
            cono->setVisible(activa);
        if (!activa)
            continue;
        cono->setRotation(torre->anguloVigilancia() * 180.f / kPi);
        const bool alerta = rocket && rocket->getVidas() > 0 && torre->detecta(rocket);
        const int tono = alerta ? 1 : 0;
        if (tono != conoTono.at(i)) {
            conoTono[i] = tono;
            cono->setBrush(brochaCono[tono]);
            cono->setPen(lapizCono[tono]);
        }
    }
}

void EscenaAsalto::actualizarObjetivos() {
    static const float arriba[3] = {-6.f, -44.f, -38.f};
    for (int i = 0; i < objetivos.size(); ++i) {
        Estructura *objetivo = objetivos.at(i);
        if (!objetivo)
            continue;
        const bool pendiente = !objetivo->estaDestruida();
        if (pendiente != flechas.at(i)->isVisible())
            flechas.at(i)->setVisible(pendiente);
        if (pendiente)
            flechas.at(i)->setPos(objetivo->getCentroX(), objetivo->getY() + arriba[i] + 5.f * std::sin(tiempo * 5.f + float(i)));
    }
}

void EscenaAsalto::actualizarHud() {
    const float restante = nivel->getTiempoRestante();
    const int segundos = int(std::ceil(restante));
    const bool resistencia = nivel->estaEnResistencia();
    const bool urgente = !resistencia && segundos <= 20;
    const bool parpadea = urgente && int(tiempo * 4.f) % 2 == 0;
    const int estadoReloj = urgente ? (parpadea ? 2 : 1) : (resistencia ? 1 : 0);
    if (resistencia != hudEnResistencia) {
        hudEnResistencia = resistencia;
        hudEtiquetaTiempo->setText(resistencia ? QStringLiteral("¡Resiste!") : QStringLiteral("Tiempo"));
        hudEtiquetaTiempo->setBrush(resistencia ? QColor(255, 170, 90) : QColor(255, 255, 255));
    }
    if (segundos != hudSegundos || estadoReloj != hudEstadoReloj) {
        if (segundos != hudSegundos) {
            hudSegundos = segundos;
            hudReloj->setText(formatoTiempo(segundos));
            hudReloj->setPos(kPanelAncho - 12.f - hudReloj->boundingRect().width(), 8);
        }
        if (estadoReloj != hudEstadoReloj) {
            hudEstadoReloj = estadoReloj;
            hudReloj->setBrush(estadoReloj == 2 ? QColor(255, 90, 70) : (estadoReloj == 1 ? QColor(255, 200, 190) : QColor(255, 255, 255)));
        }
    }

    const bool hechas[3] = {nivel->estaPolvorinVolado(), nivel->estaBanderaTumbada(), nivel->estaCanonInutilizado()};
    const int marca = (hechas[0] ? 1 : 0) | (hechas[1] ? 2 : 0) | (hechas[2] ? 4 : 0);
    if (marca != hudMarcaTareas) {
        hudMarcaTareas = marca;
        for (int i = 0; i < 3; ++i) {
            hudPunto[i]->setPen(QPen(hechas[i] ? QColor(120, 230, 140) : QColor(200, 200, 200), 2));
            hudPunto[i]->setBrush(hechas[i] ? QBrush(QColor(70, 190, 100)) : QBrush(Qt::NoBrush));
            hudTarea[i]->setBrush(hechas[i] ? QColor(150, 240, 170) : QColor(230, 230, 230));
        }
    }

    const int alarma = std::max<int>(0, std::min<int>(3, nivel->getNivelAlarma()));
    if (alarma != hudNivelAlarma) {
        hudNivelAlarma = alarma;
        static const QColor tonos[3] = {QColor(250, 214, 70), QColor(255, 150, 60), QColor(240, 70, 56)};
        for (int i = 0; i < 3; ++i)
            hudAlarma[i]->setBrush(alarma > i ? QBrush(tonos[i]) : QBrush(QColor(90, 90, 90, 210)));
    }
    panel->setPos(camX + vistaAncho - kPanelAncho - kMargen, camY + 10.f);

    const bool avisoR = nivel->getAvisoRefuerzos() > 0.f;
    if (avisoR != avisoRefuerzosVisible) {
        avisoRefuerzosVisible = avisoR;
        avisoRefuerzos->setVisible(avisoR);
    }
    if (avisoR)
        avisoRefuerzos->setPos(camX + (vistaAncho - avisoRefuerzos->boundingRect().width()) * 0.5f, camY + 64.f);

    const bool avisoE = nivel->getAvisoResistencia() > 0.f;
    if (avisoE != avisoResistenciaVisible) {
        avisoResistenciaVisible = avisoE;
        avisoResistencia->setVisible(avisoE);
    }
    if (avisoE)
        avisoResistencia->setPos(camX + (vistaAncho - avisoResistencia->boundingRect().width()) * 0.5f, camY + (avisoR ? 118.f : 64.f));
}

void EscenaAsalto::dibujarMarcador() {
    if (!nivel || !nivel->jugador() || piezasHud.vidas.isNull())
        return;
    Rocket *rocket = nivel->jugador();
    const float salud = rocket->getSaludMax() > 0.f ? rocket->getSalud() / rocket->getSaludMax() : 0.f;
    const int claveSalud = int(salud * 1000.f);
    const int claveFuria = int(rocket->getBarraFuria() * 100.f);
    const bool cambio = rocket->getVidas() != hudVidas || claveSalud != hudSalud || rocket->getMunicion() != hudMunicion
                        || rocket->getGranadas() != hudGranadas || claveFuria != hudFuria;
    if (!marcador) {
        marcador = addPixmap(QPixmap());
        marcador->setZValue(kZHud);
    }
    if (cambio) {
        const QImage lienzo = componerMarcador(piezasHud, rocket->getVidas(), salud, rocket->getMunicion(),
                                               rocket->getGranadas(), rocket->getBarraFuria());
        if (!lienzo.isNull())
            marcador->setPixmap(QPixmap::fromImage(lienzo));
        hudVidas = rocket->getVidas();
        hudSalud = claveSalud;
        hudMunicion = rocket->getMunicion();
        hudGranadas = rocket->getGranadas();
        hudFuria = claveFuria;
    }
    marcador->setPos(camX + kMargen, camY + 10.f);
}
