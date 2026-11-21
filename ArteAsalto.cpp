#include "ArteAsalto.h"

#include "AtlasNivel2.h"
#include "Enemigo.h"
#include "Entidad.h"
#include "Estructura.h"
#include "NivelAsalto.h"
#include "Proyectil.h"
#include "Rocket.h"
#include "Sprites.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QRadialGradient>
#include <QTransform>

#include <algorithm>
#include <cmath>

namespace {
const float kDisparoSoldado = 0.30f;
const float kDisparoRocket = 0.22f;
const float kLanzaRocket = 0.45f;
const float kDanioRocket = 0.40f;
const float kCaidaDuracion = 1.9f;
const float kExplosionDuracion = 1.4f;
const float kChispaDuracion = 0.14f;
const int kMaxEfectos = 48;

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
    for (int fila = 0; fila < filas; ++fila) {
        for (int col = 0; col < columnas; ++col) {
            const QPixmap celda = hoja.copy(col * ancho, fila * alto, ancho, alto)
                                      .scaled(anchoPx, altoPx, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            normal.push_back(celda);
            espejo.push_back(espejar(celda));
        }
    }
}

void dibujarImagen(QPainter &p, const QPixmap &imagen, float x, float y) {
    if (!imagen.isNull())
        p.drawPixmap(QPointF(x, y), imagen);
}
}

ArteAsalto::ArteAsalto(Sprites *sprites)
    : rocketPieX(24.f), rocketPieY(64.f), rocketAncho(48.f), soldadoPieX(20.f), soldadoPieY(60.f), soldadoAncho(40.f),
      tiempo(0.f), cuadroActual(0), punteroX(0.f), punteroY(0.f), rocketDisparoT(0.f), rocketLanzaT(0.f),
      rocketDanioT(0.f), rocketMuerteT(0.f), rocketSaludPrev(-1.f), rocketVidasPrev(-1), rocketMiraDer(true),
      hayPuntero(false), fondoAjustado(false) {
    cargar(sprites);
    efectos.reserve(kMaxEfectos);
    orden.reserve(32);
}

void ArteAsalto::cargar(Sprites *sprites) {
    const auto hoja = [sprites](const char *nombre) {
        const QPixmap *imagen = sprites ? sprites->obtener(QString::fromLatin1(nombre)) : nullptr;
        return imagen ? QPixmap(*imagen) : QPixmap();
    };

    fondo = hoja("nivel2/asalto_fondo");
    const QPixmap hojaRocket = hoja("nivel2/rocket_cenital");
    const QPixmap hojaSoldado = hoja("nivel2/soldado_cenital");
    const QPixmap escenario = hoja("nivel2/escenario");

    cortarCeldas(hojaRocket, Atlas::ROCKET_ANCHO_CELDA, Atlas::ROCKET_ALTO_CELDA, Atlas::ROCKET_COLUMNAS,
                 Atlas::ROCKET_FILAS, Atlas::ESCALA_ROCKET, rocketNormal, rocketEspejo);
    cortarCeldas(hojaSoldado, Atlas::SOLDADO_ANCHO_CELDA, Atlas::SOLDADO_ALTO_CELDA, Atlas::SOLDADO_COLUMNAS,
                 Atlas::SOLDADO_FILAS, Atlas::ESCALA_SOLDADO, soldadoNormal, soldadoEspejo);

    aspectoRocket = bloque(48, 64, QColor(70, 98, 170));
    aspectoSoldado = bloque(40, 60, QColor(176, 58, 52));
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
    }
    canonEspejo = espejar(canon);
    canonRotoEspejo = espejar(canonRoto);

    sombraSuave = pintarSombra();
    chispa = esfera(24, QColor(255, 250, 200, 255), QColor(255, 150, 40, 0));
    balaRocket = esfera(10, QColor(255, 246, 170), QColor(238, 170, 40));
    balaEnemiga = esfera(10, QColor(255, 150, 120), QColor(170, 36, 30));
    bolaCanon = esfera(18, QColor(104, 106, 112), QColor(22, 22, 26));
    granadaVuelo = esfera(16, QColor(132, 180, 90), QColor(38, 66, 30));
}

void ArteAsalto::reiniciar() {
    rastros.clear();
    efectos.clear();
    orden.clear();
    tiempo = 0.f;
    cuadroActual = 0;
    hayPuntero = false;
    rocketDisparoT = 0.f;
    rocketLanzaT = 0.f;
    rocketDanioT = 0.f;
    rocketMuerteT = 0.f;
    rocketSaludPrev = -1.f;
    rocketVidasPrev = -1;
    rocketMiraDer = true;
}

const QPixmap &ArteAsalto::celdaRocket(int fila, int col, bool espejo) const {
    const QVector<QPixmap> &lista = espejo ? rocketEspejo : rocketNormal;
    const int indice = fila * Atlas::ROCKET_COLUMNAS + col;
    if (indice < 0 || indice >= lista.size())
        return aspectoRocket;
    return lista.at(indice);
}

const QPixmap &ArteAsalto::celdaSoldado(int fila, int col, bool espejo) const {
    const QVector<QPixmap> &lista = espejo ? soldadoEspejo : soldadoNormal;
    const int indice = fila * Atlas::SOLDADO_COLUMNAS + col;
    if (indice < 0 || indice >= lista.size())
        return aspectoSoldado;
    return lista.at(indice);
}

void ArteAsalto::lanzarEfecto(short tipo, float x, float y, float radio, bool espejo) {
    if (efectos.size() >= kMaxEfectos)
        return;
    Efecto efecto;
    efecto.tipo = tipo;
    efecto.x = x;
    efecto.y = y;
    efecto.radio = radio;
    efecto.espejo = espejo;
    efectos.push_back(efecto);
}

void ArteAsalto::crearRastro(const Entidad *entidad, NivelAsalto *nivel) {
    Rastro rastro;
    rastro.clase = entidad->clase();
    rastro.x = entidad->getCentroX();
    rastro.pieY = entidad->getY() + entidad->getAlto();

    if (rastro.clase == CLASE_PROYECTIL) {
        const Proyectil *bala = static_cast<const Proyectil *>(entidad);
        rastro.bombeado = bala->esBombeado();
        rastro.deRocket = bala->esDeRocket();
        rastro.destinoX = bala->getDestinoX();
        rastro.destinoY = bala->getDestinoY();
        rastro.radio = bala->getRadioExplosion();
        rastro.x = bala->getCentroX();
        rastro.pieY = bala->getCentroY();

        if (rastro.deRocket && !rastro.bombeado) {
            rocketDisparoT = kDisparoRocket;
        } else if (rastro.deRocket) {
            rocketLanzaT = kLanzaRocket;
        } else if (!rastro.bombeado) {
            const Entidad *cercano = nullptr;
            float mejor = 1.0e12f;
            for (const Entidad *otro : nivel->obtenerEntidades()) {
                if (otro->clase() != CLASE_ENEMIGO)
                    continue;
                const float dx = otro->getCentroX() - bala->getCentroX();
                const float dy = otro->getCentroY() - bala->getCentroY();
                const float d2 = dx * dx + dy * dy;
                if (d2 < mejor) {
                    mejor = d2;
                    cercano = otro;
                }
            }
            auto it = rastros.find(cercano);
            if (cercano && it != rastros.end()) {
                it.value().disparoT = kDisparoSoldado;
                it.value().variante ^= 1;
            }
        } else if (nivel->getCanon() && nivel->jugador()) {
            const Estructura *boca = nivel->getCanon();
            const float lado = nivel->jugador()->getCentroX() > boca->getCentroX() ? 1.f : -1.f;
            lanzarEfecto(EF_EXPLOSION, boca->getCentroX() + lado * 42.f, boca->getCentroY() - 38.f, 24.f);
        }
    }

    if (rastro.clase == CLASE_ESTRUCTURA) {
        const std::vector<Estructura *> &torres = nivel->getTorres();
        rastro.variante = (!torres.empty() && torres.front() == entidad) ? 0 : 1;
    }

    rastros.insert(entidad, rastro);
}

void ArteAsalto::retirarRastro(const Rastro &rastro) {
    if (rastro.clase == CLASE_ENEMIGO)
        lanzarEfecto(EF_CAIDA, rastro.x, rastro.pieY, 0.f, rastro.espejo);
    else if (rastro.clase == CLASE_PROYECTIL) {
        if (rastro.bombeado)
            lanzarEfecto(EF_EXPLOSION, rastro.destinoX, rastro.destinoY, rastro.radio);
        else
            lanzarEfecto(EF_CHISPA, rastro.x, rastro.pieY, 0.f);
    }
}

void ArteAsalto::seguirEstructura(const Estructura *estructura, Rastro &rastro) {
    const bool rota = estructura->estaDestruida();
    if (rota && !rastro.rota) {
        const short tipo = estructura->getTipo();
        const float radio = tipo == EST_POLVORIN ? 95.f : (tipo == EST_CANON ? 60.f : 40.f);
        lanzarEfecto(EF_EXPLOSION, estructura->getCentroX(), estructura->getCentroY(), radio);
    }
    rastro.rota = rota;
}

void ArteAsalto::avanzarEntidades(NivelAsalto *nivel, float dt) {
    for (const Entidad *entidad : nivel->obtenerEntidades()) {
        auto it = rastros.find(entidad);
        if (!entidad->estaViva()) {
            if (it != rastros.end()) {
                retirarRastro(it.value());
                rastros.erase(it);
            }
            continue;
        }
        if (it != rastros.end() && it.value().clase != entidad->clase()) {
            rastros.erase(it);
            it = rastros.end();
        }
        if (it == rastros.end()) {
            crearRastro(entidad, nivel);
            it = rastros.find(entidad);
            if (it == rastros.end())
                continue;
        }
        Rastro &rastro = it.value();
        rastro.cuadro = cuadroActual;
        if (rastro.disparoT > 0.f)
            rastro.disparoT -= dt;

        if (rastro.clase == CLASE_ENEMIGO) {
            const Enemigo *enemigo = static_cast<const Enemigo *>(entidad);
            const float cx = enemigo->getCentroX();
            const float pie = enemigo->getY() + enemigo->getAlto();
            rastro.seMueve = std::fabs(cx - rastro.x) + std::fabs(pie - rastro.pieY) > 0.15f;
            rastro.x = cx;
            rastro.pieY = pie;
            bool derecha = enemigo->getDireccion() != 'L';
            const Rocket *rocket = nivel->jugador();
            if (rastro.disparoT > 0.f && rocket)
                derecha = rocket->getCentroX() >= cx;
            rastro.espejo = !derecha;
        } else if (rastro.clase == CLASE_PROYECTIL) {
            rastro.x = entidad->getCentroX();
            rastro.pieY = entidad->getCentroY();
        } else if (rastro.clase == CLASE_ESTRUCTURA) {
            seguirEstructura(static_cast<const Estructura *>(entidad), rastro);
        }
    }

    for (auto it = rastros.begin(); it != rastros.end();) {
        if (it.value().cuadro == cuadroActual) {
            ++it;
            continue;
        }
        retirarRastro(it.value());
        it = rastros.erase(it);
    }
}

void ArteAsalto::avanzarEfectos(float dt) {
    for (int i = efectos.size() - 1; i >= 0; --i) {
        efectos[i].edad += dt;
        float duracion = kExplosionDuracion;
        if (efectos[i].tipo == EF_CHISPA)
            duracion = kChispaDuracion;
        else if (efectos[i].tipo == EF_CAIDA)
            duracion = kCaidaDuracion;
        if (efectos[i].edad >= duracion)
            efectos.remove(i);
    }
}

void ArteAsalto::actualizar(NivelAsalto *nivel, float dt, float mundoX, float mundoY) {
    if (!nivel)
        return;
    if (!fondoAjustado) {
        if (!fondo.isNull())
            fondo = fondo.scaled(qRound(nivel->getAnchoMundo()), qRound(nivel->getAltoMundo()), Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation);
        fondoAjustado = true;
    }
    tiempo += dt;
    ++cuadroActual;
    punteroX = mundoX;
    punteroY = mundoY;
    hayPuntero = true;
    rocketDisparoT = std::max(0.f, rocketDisparoT - dt);
    rocketLanzaT = std::max(0.f, rocketLanzaT - dt);
    rocketDanioT = std::max(0.f, rocketDanioT - dt);

    avanzarEntidades(nivel, dt);
    avanzarEfectos(dt);

    const Rocket *rocket = nivel->jugador();
    if (!rocket)
        return;
    if (rocket->getVidas() <= 0)
        rocketMuerteT += dt;
    if (rocketVidasPrev >= 0 && (rocket->getVidas() < rocketVidasPrev || rocket->getSalud() < rocketSaludPrev - 0.01f))
        rocketDanioT = kDanioRocket;
    rocketVidasPrev = rocket->getVidas();
    rocketSaludPrev = rocket->getSalud();

    const char dir = rocket->getDireccion();
    if (dir == 'R')
        rocketMiraDer = true;
    else if (dir == 'L')
        rocketMiraDer = false;
    if ((rocketDisparoT > 0.f || rocketLanzaT > 0.f) && hayPuntero)
        rocketMiraDer = punteroX >= rocket->getCentroX();
}

void ArteAsalto::dibujarFondo(QPainter &p, const NivelAsalto *nivel) const {
    const QRectF mundo(0, 0, nivel->getAnchoMundo(), nivel->getAltoMundo());
    if (fondo.isNull())
        p.fillRect(mundo, QColor(104, 82, 54));
    else
        p.drawPixmap(mundo, fondo, QRectF(fondo.rect()));
}

void ArteAsalto::dibujarSalida(QPainter &p, const NivelAsalto *nivel) const {
    const float sx = nivel->getSalidaX();
    const float sy = nivel->getSalidaY();
    const float sr = nivel->getSalidaRadio();
    const bool lista = nivel->tareasCompletas();
    if (lista) {
        const int brillo = 120 + int(80.f * (0.5f + 0.5f * std::sin(tiempo * 5.f)));
        p.setPen(QPen(QColor(255, 214, 110, brillo + 50), 4));
        p.setBrush(QColor(226, 177, 90, brillo / 3));
    } else {
        p.setPen(QPen(QColor(226, 177, 90, 70), 3));
        p.setBrush(QColor(226, 177, 90, 18));
    }
    p.drawEllipse(QPointF(sx, sy), sr, sr);
    if (lista) {
        p.save();
        QFont fuente = p.font();
        fuente.setPixelSize(22);
        fuente.setBold(true);
        p.setFont(fuente);
        p.setPen(QColor(255, 232, 160));
        p.drawText(QRectF(sx - sr, sy - sr - 34.f, sr * 2.f, 28.f), Qt::AlignCenter, QStringLiteral("SALIDA"));
        p.restore();
    }
}

void ArteAsalto::dibujarConos(QPainter &p, const NivelAsalto *nivel) const {
    const Rocket *rocket = nivel->jugador();
    for (const Estructura *torre : nivel->getTorres()) {
        if (torre->estaDestruida())
            continue;
        const float cx = torre->getCentroX();
        const float cy = torre->getCentroY();
        const float radio = torre->getRadioCono();
        const float apertura = torre->getApertura();
        const float angulo = torre->anguloVigilancia();
        const bool alerta = rocket && rocket->getVidas() > 0 && torre->detecta(rocket);

        QPolygonF cono;
        cono << QPointF(cx, cy);
        const int tramos = 18;
        for (int i = 0; i <= tramos; ++i) {
            const float a = angulo - apertura * 0.5f + apertura * float(i) / float(tramos);
            cono << QPointF(cx + std::cos(a) * radio, cy + std::sin(a) * radio);
        }

        QRadialGradient degradado(cx, cy, radio);
        if (alerta) {
            degradado.setColorAt(0.0, QColor(255, 70, 50, 130));
            degradado.setColorAt(1.0, QColor(255, 70, 50, 34));
        } else {
            degradado.setColorAt(0.0, QColor(255, 226, 120, 96));
            degradado.setColorAt(1.0, QColor(255, 226, 120, 14));
        }
        p.setPen(QPen(alerta ? QColor(255, 90, 70, 150) : QColor(255, 226, 120, 120), 1.5));
        p.setBrush(degradado);
        p.drawPolygon(cono);
    }
}

void ArteAsalto::dibujarAvisos(QPainter &p) const {
    for (auto it = rastros.constBegin(); it != rastros.constEnd(); ++it) {
        const Rastro &rastro = it.value();
        if (rastro.clase != CLASE_PROYECTIL || !rastro.bombeado)
            continue;
        const float radio = std::max(10.f, rastro.radio);
        const QColor color = rastro.deRocket ? QColor(240, 230, 140) : QColor(236, 74, 50);
        p.setPen(QPen(QColor(color.red(), color.green(), color.blue(), 190), 2));
        p.setBrush(QColor(color.red(), color.green(), color.blue(), 38));
        p.drawEllipse(QPointF(rastro.destinoX, rastro.destinoY), radio, radio);
    }
}

void ArteAsalto::dibujarSombra(QPainter &p, float cx, float pie) const {
    dibujarImagen(p, sombraSuave, cx - 26.f, pie - 9.f);
}

void ArteAsalto::dibujarRocket(QPainter &p, const Rocket *rocket) const {
    const float cx = rocket->getCentroX();
    const float pie = rocket->getY() + rocket->getAlto();
    const char dir = rocket->getDireccion();
    const bool vivo = rocket->getVidas() > 0;

    int fila = Atlas::ROCKET_REPOSO;
    int col = int(tiempo * 4.f) % Atlas::ROCKET_COLUMNAS;
    if (!vivo) {
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

    dibujarSombra(p, cx, pie);
    if (rocket->estaFurioso() && vivo) {
        p.setPen(QPen(QColor(255, 90, 60, 170), 2));
        p.setBrush(QColor(255, 70, 40, 60));
        p.setOpacity(0.65 + 0.35 * std::sin(tiempo * 9.f));
        p.drawEllipse(QPointF(cx, pie - 6.f), 48.0, 20.0);
    }

    const bool espejo = !rocketMiraDer;
    const float pieX = espejo ? rocketAncho - rocketPieX : rocketPieX;
    p.setOpacity(!rocket->esInvulnerable() || std::fmod(tiempo, 0.24f) < 0.12f ? 1.0 : 0.35);
    dibujarImagen(p, celdaRocket(fila, col, espejo), cx - pieX, pie - rocketPieY);
    p.setOpacity(1.0);
}

void ArteAsalto::dibujarSoldado(QPainter &p, const Enemigo *enemigo) const {
    const Rastro rastro = rastros.value(enemigo);
    const float cx = enemigo->getCentroX();
    const float pie = enemigo->getY() + enemigo->getAlto();

    int fila = Atlas::SOLDADO_POSES;
    int col = int(tiempo * 2.f) % 3;
    if (rastro.disparoT > 0.f) {
        fila = rastro.variante ? Atlas::SOLDADO_DISPARA_1 : Atlas::SOLDADO_DISPARA_0;
        col = std::max(0, std::min(5, int((kDisparoSoldado - rastro.disparoT) * 20.f)));
    } else if (rastro.seMueve || enemigo->estadoVisual() != 0) {
        fila = enemigo->estadoVisual() == 0 ? Atlas::SOLDADO_CAMINA_2 : Atlas::SOLDADO_CAMINA_3;
        col = int(tiempo * (enemigo->estadoVisual() == 0 ? 8.f : 12.f)) % Atlas::SOLDADO_COLUMNAS;
    }

    const bool espejo = rastro.espejo;
    const float pieX = espejo ? soldadoAncho - soldadoPieX : soldadoPieX;
    dibujarSombra(p, cx, pie);
    dibujarImagen(p, celdaSoldado(fila, col, espejo), cx - pieX, pie - soldadoPieY);
}

const QPixmap &ArteAsalto::imagenEstructura(const NivelAsalto *nivel, const Estructura *estructura, float &desfase) const {
    const bool rota = estructura->estaDestruida();
    desfase = 0.f;
    switch (estructura->getTipo()) {
    case EST_TORRE:
        return rastros.value(estructura).variante == 0 ? torreA : torreB;
    case EST_POLVORIN:
        return rota ? polvorinRoto : polvorin;
    case EST_BANDERA:
        return rota ? banderaCaida : bandera;
    case EST_CANON: {
        const Rocket *rocket = nivel->jugador();
        const bool espejo = rocket && rocket->getCentroX() > estructura->getCentroX();
        desfase = 6.f;
        if (rota)
            return espejo ? canonRotoEspejo : canonRoto;
        return espejo ? canonEspejo : canon;
    }
    default:
        return aspectoSoldado;
    }
}

void ArteAsalto::dibujarEstructura(QPainter &p, const NivelAsalto *nivel, const Estructura *estructura) const {
    float desfase = 0.f;
    const QPixmap &imagen = imagenEstructura(nivel, estructura, desfase);
    const float cx = estructura->getCentroX();
    const float base = estructura->getY() + estructura->getAlto();
    dibujarImagen(p, imagen, cx - imagen.width() / 2.f, base + desfase - imagen.height());
}

void ArteAsalto::dibujarHumo(QPainter &p, const NivelAsalto *nivel, const Estructura *estructura) const {
    const short tipo = estructura->getTipo();
    if (!estructura->estaDestruida() || (tipo != EST_POLVORIN && tipo != EST_CANON) || humoDestruccion.isNull())
        return;
    float desfase = 0.f;
    const QPixmap &imagen = imagenEstructura(nivel, estructura, desfase);
    const float cx = estructura->getCentroX();
    const float base = estructura->getY() + estructura->getAlto();
    const float oscila = std::sin(tiempo * 1.7f + cx);
    p.setOpacity(0.48 + 0.12 * oscila);
    dibujarImagen(p, humoDestruccion, cx - humoDestruccion.width() / 2.f + oscila * 5.f,
                  base - imagen.height() * 0.75f - humoDestruccion.height() * 0.5f - 6.f * oscila);
    p.setOpacity(1.0);
}

void ArteAsalto::dibujarProyectiles(QPainter &p, const NivelAsalto *nivel) const {
    for (const Entidad *entidad : nivel->obtenerEntidades()) {
        if (entidad->clase() != CLASE_PROYECTIL || !entidad->estaViva())
            continue;
        const Proyectil *bala = static_cast<const Proyectil *>(entidad);
        const QPixmap &imagen = bala->esBombeado() ? (bala->esDeRocket() ? granadaVuelo : bolaCanon)
                                                   : (bala->esDeRocket() ? balaRocket : balaEnemiga);
        dibujarImagen(p, imagen, bala->getCentroX() - imagen.width() / 2.f, bala->getCentroY() - imagen.height() / 2.f);
    }
}

void ArteAsalto::dibujarEfectos(QPainter &p, bool cuerpos) const {
    for (const Efecto &efecto : efectos) {
        const bool esCuerpo = efecto.tipo == EF_CAIDA;
        if (esCuerpo != cuerpos)
            continue;
        const float t = efecto.edad;

        if (esCuerpo) {
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
            const float pieX = efecto.espejo ? soldadoAncho - soldadoPieX : soldadoPieX;
            p.setOpacity(t < 1.3f ? 1.0 : std::max(0.0, 1.0 - (t - 1.3) / 0.6));
            dibujarImagen(p, celdaSoldado(fila, col, efecto.espejo), efecto.x - pieX, efecto.y - soldadoPieY);
            p.setOpacity(1.0);
            continue;
        }

        const QPixmap *imagen = &chispa;
        float escala = 1.f;
        float opacidad = 1.f;
        float subida = 0.f;
        if (efecto.tipo == EF_CHISPA) {
            escala = 0.6f + 0.8f * t / kChispaDuracion;
            opacidad = 1.f - t / kChispaDuracion;
        } else {
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
        }
        p.save();
        p.setOpacity(std::max(0.f, opacidad));
        p.translate(efecto.x, efecto.y - subida);
        p.scale(escala, escala);
        p.drawPixmap(QPointF(-imagen->width() / 2.0, -imagen->height() / 2.0), *imagen);
        p.restore();
    }
}

void ArteAsalto::dibujarFlechas(QPainter &p, const NivelAsalto *nivel) const {
    const Estructura *objetivos[3] = {nivel->getPolvorin(), nivel->getBandera(), nivel->getCanon()};
    static const float arriba[3] = {-6.f, -44.f, -38.f};
    p.setPen(QPen(QColor(50, 28, 10), 2));
    p.setBrush(QColor(255, 208, 70));
    for (int i = 0; i < 3; ++i) {
        const Estructura *objetivo = objetivos[i];
        if (!objetivo || objetivo->estaDestruida())
            continue;
        const float x = objetivo->getCentroX();
        const float y = objetivo->getY() + arriba[i] + 5.f * std::sin(tiempo * 5.f + i);
        QPolygonF flecha;
        flecha << QPointF(x - 11, y - 18) << QPointF(x + 11, y - 18) << QPointF(x, y);
        p.drawPolygon(flecha);
    }
}

void ArteAsalto::dibujar(QPainter &p, NivelAsalto *nivel) {
    if (!nivel)
        return;
    p.save();
    dibujarFondo(p, nivel);
    dibujarSalida(p, nivel);
    dibujarConos(p, nivel);
    dibujarAvisos(p);
    dibujarEfectos(p, true);

    orden.clear();
    for (const Entidad *entidad : nivel->obtenerEntidades()) {
        const short clase = entidad->clase();
        if (clase != CLASE_ROCKET && clase != CLASE_ENEMIGO && clase != CLASE_ESTRUCTURA)
            continue;
        if (!entidad->estaViva() && clase != CLASE_ROCKET)
            continue;
        orden.push_back({entidad->getY() + entidad->getAlto(), entidad});
    }
    std::sort(orden.begin(), orden.end(), [](const Orden &a, const Orden &b) { return a.y < b.y; });

    for (const Orden &item : orden) {
        switch (item.entidad->clase()) {
        case CLASE_ROCKET:
            dibujarRocket(p, static_cast<const Rocket *>(item.entidad));
            break;
        case CLASE_ENEMIGO:
            dibujarSoldado(p, static_cast<const Enemigo *>(item.entidad));
            break;
        default:
            dibujarEstructura(p, nivel, static_cast<const Estructura *>(item.entidad));
            break;
        }
    }
    for (const Orden &item : orden)
        if (item.entidad->clase() == CLASE_ESTRUCTURA)
            dibujarHumo(p, nivel, static_cast<const Estructura *>(item.entidad));

    dibujarProyectiles(p, nivel);
    dibujarEfectos(p, false);
    dibujarFlechas(p, nivel);
    p.restore();
}
