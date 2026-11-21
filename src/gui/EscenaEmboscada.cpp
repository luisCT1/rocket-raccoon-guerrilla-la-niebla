#include "EscenaEmboscada.h"

#include "Enemigo.h"
#include "Estructura.h"
#include "Eventos.h"
#include "HojaSprites.h"
#include "Juego.h"
#include "NivelEmboscada.h"
#include "Proyectil.h"
#include "Rocket.h"

#include <QFont>
#include <QImage>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QPainter>

#include <algorithm>
#include <cmath>
#include <initializer_list>

namespace {
const float kVistaAncho = 1280.f;
const float kVistaAlto = 720.f;

QPixmap voltear(const QPixmap &cuadro) {
    return cuadro.transformed(QTransform().scale(-1, 1), Qt::SmoothTransformation);
}

QPixmap circulo(const QColor &color, int diametro) {
    QPixmap mapa(diametro, diametro);
    mapa.fill(Qt::transparent);
    QPainter pintor(&mapa);
    pintor.setRenderHint(QPainter::Antialiasing);
    pintor.setBrush(color);
    pintor.setPen(Qt::NoPen);
    pintor.drawEllipse(1, 1, diametro - 2, diametro - 2);
    return mapa;
}

QImage sinRotulo(QImage img) {
    auto esRotulo = [](QRgb p) {
        if (qAlpha(p) < 28)
            return false;
        const int r = qRed(p);
        const int g = qGreen(p);
        const int b = qBlue(p);
        if (r > 210 && g > 210 && b > 210)
            return true;
        return b > r + 12 && b > 60 && r < 140 && g < 180;
    };
    int x0 = 0;
    while (x0 < img.width() / 2) {
        int rotulo = 0;
        int otro = 0;
        for (int y = 0; y < img.height(); ++y) {
            const QRgb p = img.pixel(x0, y);
            if (qAlpha(p) < 28)
                continue;
            const int r = qRed(p);
            const int g = qGreen(p);
            const int b = qBlue(p);
            if (r < 45 && g < 45 && b < 45)
                continue;
            if (esRotulo(p))
                ++rotulo;
            else
                ++otro;
        }
        if (rotulo > 0 && rotulo >= otro) {
            ++x0;
            continue;
        }
        if (rotulo == 0 && otro == 0) {
            ++x0;
            continue;
        }
        break;
    }
    if (x0 > 4)
        img = img.copy(x0, 0, img.width() - x0, img.height());

    int y0 = 0;
    while (y0 < img.height() / 2) {
        int rotulo = 0;
        int otro = 0;
        for (int x = 0; x < img.width(); ++x) {
            const QRgb p = img.pixel(x, y0);
            if (qAlpha(p) < 28)
                continue;
            const int r = qRed(p);
            const int g = qGreen(p);
            const int b = qBlue(p);
            if (r < 45 && g < 45 && b < 45)
                continue;
            if (esRotulo(p))
                ++rotulo;
            else
                ++otro;
        }
        if ((rotulo > 0 && rotulo >= otro) || (rotulo == 0 && otro == 0)) {
            ++y0;
            continue;
        }
        break;
    }
    if (y0 > 4)
        img = img.copy(0, y0, img.width(), img.height() - y0);
    return img;
}

QPixmap ajustar(const QImage &img) {
    int x0 = img.width();
    int y0 = img.height();
    int x1 = -1;
    int y1 = -1;
    for (int yy = 0; yy < img.height(); ++yy) {
        for (int xx = 0; xx < img.width(); ++xx) {
            if (qAlpha(img.pixel(xx, yy)) < 28)
                continue;
            if (xx < x0) x0 = xx;
            if (yy < y0) y0 = yy;
            if (xx > x1) x1 = xx;
            if (yy > y1) y1 = yy;
        }
    }
    if (x1 < x0)
        return {};
    return QPixmap::fromImage(img.copy(x0, y0, x1 - x0 + 1, y1 - y0 + 1));
}

QVector<QPixmap> cuadros(const QPixmap &hoja, std::initializer_list<QRect> zonas, bool quitarRotulo = true) {
    QVector<QPixmap> salida;
    if (hoja.isNull())
        return salida;
    for (const QRect &zona : zonas) {
        QImage img = hoja.copy(zona).toImage().convertToFormat(QImage::Format_ARGB32);
        if (quitarRotulo)
            img = sinRotulo(img);
        const QPixmap cuadro = ajustar(img);
        if (!cuadro.isNull() && cuadro.width() >= 36 && cuadro.height() >= 36)
            salida.push_back(cuadro);
    }
    return salida;
}

int lineaApoyo(const QPixmap &cuadro, float fraccion = 0.55f) {
    const QImage img = cuadro.toImage();
    const int ancho = img.width();
    const int necesario = qMax(6, int(ancho * fraccion));
    for (int y = img.height() - 1; y >= 0; --y) {
        int n = 0;
        int x0 = ancho;
        int x1 = -1;
        for (int x = 0; x < ancho; ++x) {
            if (qAlpha(img.pixel(x, y)) <= 24)
                continue;
            ++n;
            if (x < x0) x0 = x;
            if (x > x1) x1 = x;
        }
        if (n >= 6 && (x1 - x0 + 1) >= necesario)
            return y;
    }
    return qMax(0, img.height() - 1);
}

QVector<QPixmap> separar(const QPixmap &hoja, int y0, int y1, int altoMin, int xMin) {
    QVector<QPixmap> salida;
    if (hoja.isNull())
        return salida;
    const QImage img = hoja.toImage().convertToFormat(QImage::Format_ARGB32);
    y0 = qBound(0, y0, img.height() - 1);
    y1 = qBound(y0, y1, img.height() - 1);
    xMin = qBound(0, xMin, img.width() - 1);
    const int w = img.width();
    const int alto = y1 - y0 + 1;
    QVector<int> visto(w * alto, 0);
    QVector<int> cola;
    int marca = 0;
    for (int y = y0; y <= y1; ++y) {
        for (int x = xMin; x < w; ++x) {
            const int origen = (y - y0) * w + x;
            if (visto.at(origen) || qAlpha(img.pixel(x, y)) < 28)
                continue;
            ++marca;
            int bx0 = x;
            int by0 = y;
            int bx1 = x;
            int by1 = y;
            cola.clear();
            cola.append(origen);
            visto[origen] = marca;
            for (int k = 0; k < cola.size(); ++k) {
                const int i = cola.at(k);
                const int px = i % w;
                const int py = y0 + i / w;
                bx0 = std::min(bx0, px);
                by0 = std::min(by0, py);
                bx1 = std::max(bx1, px);
                by1 = std::max(by1, py);
                const int dx[4] = {-1, 1, 0, 0};
                const int dy[4] = {0, 0, -1, 1};
                for (int v = 0; v < 4; ++v) {
                    const int nx = px + dx[v];
                    const int ny = py + dy[v];
                    if (nx < xMin || ny < y0 || nx >= w || ny > y1)
                        continue;
                    const int ni = (ny - y0) * w + nx;
                    if (visto.at(ni) || qAlpha(img.pixel(nx, ny)) < 28)
                        continue;
                    visto[ni] = marca;
                    cola.append(ni);
                }
            }
            if (by1 - by0 + 1 < altoMin)
                continue;
            QImage recorte(bx1 - bx0 + 1, by1 - by0 + 1, QImage::Format_ARGB32);
            recorte.fill(Qt::transparent);
            for (int py = by0; py <= by1; ++py) {
                for (int px = bx0; px <= bx1; ++px) {
                    if (visto.at((py - y0) * w + px) == marca)
                        recorte.setPixel(px - bx0, py - by0, img.pixel(px, py));
                }
            }
            salida.push_back(QPixmap::fromImage(recorte));
        }
    }
    return salida;
}

struct TrozoSelva {
    QPixmap dibujo;
    int orden = 0;
};

QVector<TrozoSelva> recortarSelva(const QPixmap &hoja) {
    QVector<TrozoSelva> salida;
    const QVector<QPixmap> piezas = separar(hoja, 0, hoja.isNull() ? 0 : hoja.height() - 1, 36, 0);
    int orden = 0;
    for (const QPixmap &pieza : piezas) {
        if (pieza.width() > 620 || pieza.width() < 28 || pieza.height() < 28)
            continue;
        int tope = 84;
        if (pieza.height() > pieza.width() && pieza.height() > 220)
            tope = 200;
        else if (pieza.height() > 160)
            tope = 120;
        QPixmap lista = pieza;
        if (lista.height() > tope)
            lista = lista.scaledToHeight(tope, Qt::SmoothTransformation);
        salida.append({lista, orden++});
    }
    return salida;
}

}

EscenaEmboscada::EscenaEmboscada() {
    setSceneRect(0, 0, 4000, 720);
    setBackgroundBrush(QColor(168, 186, 190));
    cargarSprites();

    juego = new Juego();
    juego->iniciarNivel(NIVEL_EMBOSCADA, DIF_NORMAL);
    nivel = dynamic_cast<NivelEmboscada *>(juego->nivelActivo());
    armarSuelo();
}

EscenaEmboscada::~EscenaEmboscada() {
    delete juego;
}

void EscenaEmboscada::cargarSprites() {
    const QPixmap rocket(QStringLiteral(":/nivel1/sprites_rocket.png"));
    caminar = cuadros(rocket, {QRect(18, 36, 153, 139), QRect(188, 36, 145, 139), QRect(353, 36, 147, 139),
                               QRect(521, 36, 144, 139), QRect(691, 36, 134, 139), QRect(847, 36, 147, 139)}, false);
    espera = cuadros(rocket, {QRect(18, 36, 153, 139)}, false);
    saltar = cuadros(rocket, {QRect(17, 1159, 144, 138), QRect(186, 1159, 151, 138), QRect(367, 1159, 121, 138),
                              QRect(522, 1159, 140, 138), QRect(686, 1159, 143, 138), QRect(862, 1159, 144, 138)}, false);
    disparar = cuadros(rocket, {QRect(20, 406, 162, 139), QRect(189, 406, 143, 139), QRect(356, 406, 167, 139),
                                QRect(527, 406, 171, 139), QRect(699, 406, 135, 139), QRect(852, 406, 148, 139)}, false);
    lanzar = cuadros(rocket, {QRect(368, 595, 167, 141), QRect(542, 595, 121, 141), QRect(699, 595, 131, 141)}, false);
    impulso = cuadros(rocket, {QRect(14, 783, 157, 150), QRect(182, 783, 161, 150),
                               QRect(355, 783, 149, 150), QRect(527, 783, 144, 150)}, false);
    furia = cuadros(rocket, {QRect(15, 1340, 164, 155), QRect(198, 1340, 140, 155), QRect(522, 1340, 140, 155)}, false);
    rocketDanio = cuadros(rocket, {QRect(522, 960, 148, 151), QRect(684, 960, 147, 151)}, false);
    rocketMuerte = cuadros(rocket, {QRect(684, 960, 147, 151), QRect(851, 960, 154, 151)}, false);

    const QPixmap soldados(QStringLiteral(":/nivel1/sprites_soldados.png"));
    soldadoCamina = cuadros(soldados, {QRect(62, 39, 132, 177), QRect(320, 39, 126, 177), QRect(574, 39, 131, 177),
                                       QRect(828, 39, 135, 177), QRect(1083, 39, 137, 177), QRect(1334, 39, 148, 177)}, false);
    soldadoDispara = cuadros(soldados, {QRect(58, 1073, 140, 157), QRect(268, 1073, 232, 157), QRect(594, 1073, 91, 157),
                                        QRect(858, 1073, 76, 157), QRect(1099, 1073, 106, 157), QRect(1360, 1073, 96, 157)}, false);
    soldadoMuere = cuadros(soldados, {QRect(59, 1588, 138, 151), QRect(329, 1588, 109, 151), QRect(587, 1588, 105, 151),
                                      QRect(824, 1588, 144, 151), QRect(1082, 1588, 140, 151), QRect(1327, 1588, 162, 151)}, false);
    soldadoBandera = cuadros(soldados, {QRect(1091, 1817, 121, 205)}, false);

    const QPixmap jinete(QStringLiteral(":/nivel1/sprites_jinete.png"));
    jineteGalope = cuadros(jinete, {QRect(103, 726, 192, 199), QRect(506, 726, 186, 199), QRect(908, 726, 184, 199),
                                    QRect(1308, 726, 184, 199), QRect(1706, 726, 188, 199), QRect(2108, 726, 184, 199)}, false);
    jineteBandera = cuadros(jinete, {QRect(126, 3356, 148, 216), QRect(523, 3356, 154, 216)}, false);
    jineteTrompeta = cuadros(jinete, {QRect(917, 3356, 166, 216), QRect(1327, 3356, 146, 216),
                                      QRect(1732, 3356, 136, 216), QRect(2126, 3356, 148, 216)}, false);
    jineteMuere = cuadros(jinete, {QRect(105, 3020, 190, 230), QRect(508, 3020, 184, 230), QRect(922, 3020, 154, 230)}, false);

    const QPixmap convoy(QStringLiteral(":/nivel1/sprites_convoy.png"));
    carretaRueda = cuadros(convoy, {QRect(20, 8, 265, 132), QRect(297, 8, 246, 132), QRect(553, 8, 222, 132),
                                    QRect(786, 8, 220, 132), QRect(1020, 8, 224, 132), QRect(1257, 8, 234, 132)});
    carretaHerida = cuadros(convoy, {QRect(24, 291, 256, 143), QRect(289, 291, 261, 143), QRect(558, 291, 231, 143),
                                     QRect(802, 291, 235, 143), QRect(1052, 291, 211, 143), QRect(1278, 291, 227, 143)});
    carretaExplota = cuadros(convoy, {QRect(22, 710, 195, 141), QRect(239, 710, 207, 141), QRect(454, 710, 207, 141),
                                      QRect(663, 710, 209, 141), QRect(877, 710, 199, 141), QRect(1080, 710, 198, 141)});
    barrilQuieto = cuadros(convoy, {QRect(24, 448, 80, 110), QRect(130, 448, 70, 110), QRect(228, 448, 85, 110)});
    barrilEstalla = separar(convoy, 448, 640, 40, 520);

    const QVector<QPixmap> puentes = cortarHoja(QPixmap(QStringLiteral(":/nivel1/puente_oscilante.png")),
        {{569, 624, 519, 236}}, 0);
    if (!puentes.isEmpty())
        puenteQuieto = puentes.first();

    prepararHud(QPixmap(QStringLiteral(":/nivel1/sprint_vidas_municion.png")));
}

void EscenaEmboscada::apoyar(QGraphicsPixmapItem *sprite, const QPixmap &cuadro, float centro, float suelo, float alto, float altoFuente, float fraccion) {
    if (!sprite || cuadro.isNull())
        return;
    const float ref = altoFuente > 1.f ? altoFuente : float(qMax(1, cuadro.height()));
    const float factor = alto / ref;
    const QPixmap puesto = cuadro.scaledToHeight(qMax(1, qRound(cuadro.height() * factor)), Qt::SmoothTransformation);
    sprite->setPixmap(puesto);
    sprite->setPos(centro - puesto.width() / 2.f, suelo - float(lineaApoyo(cuadro, fraccion) + 1) * factor);
}

void EscenaEmboscada::prepararHud(const QPixmap &hoja) {
    if (!hoja.isNull())
        prepararMarcador(hoja.toImage().convertToFormat(QImage::Format_ARGB32), piezasHud);
    return;
}

void EscenaEmboscada::dibujarHud() {
    if (!nivel || !nivel->jugador() || piezasHud.vidas.isNull())
        return;
    Rocket *rocket = nivel->jugador();
    const QImage lienzo = componerMarcador(piezasHud, rocket->getVidas(),
        rocket->getSaludMax() > 0.f ? rocket->getSalud() / rocket->getSaludMax() : 0.f,
        rocket->getMunicion(), rocket->getGranadas(), rocket->getBarraFuria());
    if (lienzo.isNull())
        return;
    if (!marcador) {
        marcador = addPixmap(QPixmap());
        marcador->setZValue(40);
    }
    marcador->setPixmap(QPixmap::fromImage(lienzo));
    marcador->setPos((nivel ? nivel->getCamaraX() : 0.f) + 14.f, 8.f);
}

void EscenaEmboscada::armarSuelo() {
    const QPixmap cielo(QStringLiteral(":/nivel1/cielo.png"));
    const QPixmap camino(QStringLiteral(":/nivel1/camino.png"));
    const QPixmap avismo(QStringLiteral(":/nivel1/avismo.png"));
    const QPixmap selva(QStringLiteral(":/nivel1/selva.png"));
    const QPixmap niebla(QStringLiteral(":/nivel1/niebla.png"));

    if (!cielo.isNull())
        setBackgroundBrush(cielo.toImage().pixelColor(160, 40));

    auto poner = [this](const QPixmap &pieza, float x, float y, qreal z, float avance,
                        float deriva = 0.f, float ciclo = 0.f) {
        if (pieza.isNull())
            return;
        auto *trozo = addPixmap(pieza);
        trozo->setPos(x, y);
        trozo->setZValue(z);
        capas.append({trozo, x, y, avance, deriva, ciclo});
    };

    if (!cielo.isNull()) {
        const int alto = 720;
        const int ancho = qMax(1, qRound(cielo.width() * (float(alto) / cielo.height())));
        const QPixmap lejos = cielo.scaled(ancho, alto, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        poner(lejos, kVistaAncho - lejos.width(), 0.f, 0.02, 0.f);
    }

    if (!niebla.isNull()) {
        const QPixmap banda = ajustar(niebla.toImage().convertToFormat(QImage::Format_ARGB32)).scaledToWidth(1680, Qt::SmoothTransformation);
        const float yNiebla = 300.f;
        for (float x = -float(banda.width()); x <= float(banda.width()); x += banda.width()) {
            auto *trozo = addPixmap(banda);
            trozo->setOpacity(0.9);
            trozo->setPos(x, yNiebla);
            trozo->setZValue(0.12);
            capas.append({trozo, x, yNiebla, 0.f, 28.f, float(banda.width())});
        }
    }

    const float piso = nivel ? nivel->getSuelo() : 548.f;
    const float filaCamino = 316.f;
    const float filaAvismo = 324.f;
    const float baseCamino = 508.f;
    const float escala = (712.f - piso) / (baseCamino - filaCamino);
    const float yCamino = piso - filaCamino * escala;
    const float yAvismo = piso - filaAvismo * escala;
    auto escalar = [escala](const QPixmap &pieza) {
        return pieza.scaled(qMax(1, qRound(pieza.width() * escala)),
                            qMax(1, qRound(pieza.height() * escala)),
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    };

    const int corte = 1000;
    const int labio = 1514;

    if (!avismo.isNull() && !camino.isNull()) {
        const QPixmap borde = escalar(avismo.copy(corte, 0, avismo.width() - corte, avismo.height()));
        const QPixmap vuelto = borde.transformed(QTransform().scale(-1, 1), Qt::FastTransformation);
        const float labioLocal = (labio - corte) * escala;
        const float xIzq = 1500.f - labioLocal;
        const float xDer = 1920.f - (borde.width() - labioLocal);

        const int largoIzq = qMin(camino.width(), qRound((xIzq + 40.f) / escala));
        poner(escalar(camino.copy(0, 0, largoIzq, camino.height())), 0.f, yCamino, 0.4, 1.f);
        poner(borde, xIzq, yAvismo, 0.46, 1.f);
        poner(vuelto, xDer, yAvismo, 0.46, 1.f);
        poner(escalar(camino), xDer + borde.width() - 46.f, yCamino, 0.4, 1.f);
    }

    const float puestos[] = { 30.f, 380.f, 720.f, 1080.f, 2050.f, 2420.f, 2780.f, 3180.f, 3520.f, 3820.f };
    const QVector<TrozoSelva> piezas = recortarSelva(selva);
    const int cupo = qMin(10, piezas.size());
    for (int i = 0; i < cupo; ++i) {
        const int cual = piezas.size() <= 10 ? i : i * piezas.size() / cupo;
        const QPixmap &pieza = piezas.at(cual).dibujo;
        poner(pieza, puestos[i], piso - pieza.height() + 6.f, 0.62, 1.f);
    }
}

void EscenaEmboscada::moverCapas() {
    const float cam = nivel ? nivel->getCamaraX() : 0.f;
    for (const CapaFondo &capa : capas) {
        if (!capa.dibujo)
            continue;
        float x = capa.x + cam * (1.f - capa.avance);
        if (capa.ciclo > 1.f) {
            float d = std::fmod(tiempo * capa.deriva, capa.ciclo);
            if (d < 0.f)
                d += capa.ciclo;
            x += d;
        }
        capa.dibujo->setPos(x, capa.y);
    }
}

void EscenaEmboscada::pulsar(int tecla) {
    teclas.insert(tecla);
    if (!nivel || nivel->estaTerminado() || !nivel->jugador())
        return;

    if (tecla == Qt::Key_W || tecla == Qt::Key_Up)
        nivel->jugadorSalta();
    if (tecla == Qt::Key_Shift)
        nivel->jugadorDash();
    if (tecla == Qt::Key_F)
        nivel->jugadorActivaFuria();
    if (tecla == Qt::Key_G) {
        Rocket *rocket = nivel->jugador();
        const float dir = rocket->getDireccion() == 'L' ? -1.f : 1.f;
        nivel->jugadorLanzaGranada(rocket->getCentroX() + dir * 260.f, rocket->getY() + rocket->getAlto() - 4.f);
        granadaMuestra = 0.45f;
    }
}

void EscenaEmboscada::soltar(int tecla) {
    teclas.remove(tecla);
}

void EscenaEmboscada::apuntar(float x, float y) {
    punteroX = x;
    punteroY = y;
    hayApunte = true;
}

void EscenaEmboscada::avanzar(float dt) {
    if (!nivel)
        return;

    tiempo += dt;
    if (granadaMuestra > 0.f)
        granadaMuestra = qMax(0.f, granadaMuestra - dt);

    Rocket *rocket = nivel->jugador();
    if (rocket && !nivel->estaTerminado()) {
        if (teclas.contains(Qt::Key_A) || teclas.contains(Qt::Key_Left))
            nivel->jugadorMueve('L');
        if (teclas.contains(Qt::Key_D) || teclas.contains(Qt::Key_Right))
            nivel->jugadorMueve('R');

        const float dir = rocket->getDireccion() == 'L' ? -1.f : 1.f;
        const float ax = rocket->getCentroX() + dir * 480.f;
        const float ay = rocket->getY() + rocket->getAlto() * 0.35f;
        if (teclas.contains(Qt::Key_Space))
            nivel->jugadorDispara(ax, ay);
    }

    QHash<Entidad *, QPointF> estructuras;
    for (Entidad *entidad : nivel->obtenerEntidades()) {
        if (entidad->clase() != CLASE_ESTRUCTURA)
            continue;
        if (static_cast<Estructura *>(entidad)->getTipo() == EST_PUENTE)
            continue;
        estructuras.insert(entidad, QPointF(entidad->getCentroX(), entidad->getCentroY()));
    }

    juego->actualizar(dt);
    moverCapas();
    sincronizar();

    const std::vector<Entidad *> &vivas = nivel->obtenerEntidades();
    QSet<Entidad *> siguen;
    for (Entidad *entidad : vivas)
        siguen.insert(entidad);
    for (auto marca = estructuras.cbegin(); marca != estructuras.cend(); ++marca) {
        if (siguen.contains(marca.key()))
            continue;
        const QPointF punto = marca.value();
        auto *nube = addEllipse(punto.x() - 46.f, punto.y() - 30.f, 92, 60,
            QPen(QColor(90, 70, 40)), QBrush(QColor(176, 120, 58, 150)));
        nube->setZValue(5);
        flashes.append(nube);
        vidaFlash.append(0.28f);
    }

    for (int i = flashes.size() - 1; i >= 0; --i) {
        vidaFlash[i] -= dt;
        if (vidaFlash[i] <= 0.f) {
            removeItem(flashes[i]);
            delete flashes[i];
            flashes.removeAt(i);
            vidaFlash.removeAt(i);
        }
    }

    for (int i = caidas.size() - 1; i >= 0; --i) {
        Caida &caida = caidas[i];
        caida.edad += dt;
        if (caida.cuadros.isEmpty() || caida.edad > caida.cuadros.size() * 0.12f + 0.4f) {
            removeItem(caida.item);
            delete caida.item;
            caidas.removeAt(i);
            continue;
        }
        const int frame = qMin(int(caida.edad * 8.f), caida.cuadros.size() - 1);
        apoyar(caida.item, caida.cuadros.at(frame), caida.centro, caida.suelo, caida.alto, caida.fuente, 0.22f);
    }
}

QRectF EscenaEmboscada::rectCamara() const {
    float x = nivel ? nivel->getCamaraX() : 0.f;
    return QRectF(x, 0, kVistaAncho, kVistaAlto);
}

QString EscenaEmboscada::textoHud() const {
    return {};
}

QString EscenaEmboscada::mensajeFinal() const {
    if (!nivel || !nivel->estaTerminado() || !nivel->jugador())
        return {};
    if (nivel->fueGanado())
        return QStringLiteral("Misión cumplida\nEl convoy no pasó y el correo no avisó.");
    if (nivel->getCorreoEscapo())
        return QStringLiteral("Misión fallida\nEl correo llevó el aviso.");
    if (nivel->jugador()->cayoAlVacio())
        return QStringLiteral("Misión fallida\nCaíste al barranco y te quedaste sin oportunidades.");
    return QStringLiteral("Misión fallida\nTe mataron.");
}

int EscenaEmboscada::porcentajeVida() const {
    if (!nivel || !nivel->jugador() || nivel->jugador()->getSaludMax() <= 0.f)
        return 0;
    const float parte = nivel->jugador()->getSalud() / nivel->jugador()->getSaludMax();
    return qBound(0, qRound(parte * 100.f), 100);
}

bool EscenaEmboscada::acabo() const {
    return nivel && nivel->estaTerminado();
}

bool EscenaEmboscada::gano() const {
    return nivel && nivel->fueGanado();
}

bool EscenaEmboscada::miraALaIzquierda(Entidad *entidad) const {
    if (auto *rocket = dynamic_cast<Rocket *>(entidad))
        return rocket->getDireccion() == 'L';
    if (auto *enemigo = dynamic_cast<Enemigo *>(entidad))
        return enemigo->getDireccion() == 'L';
    return false;
}

QPixmap EscenaEmboscada::cuadroDe(Entidad *entidad) const {
    const QVector<QPixmap> *lista = nullptr;
    int indice = 0;

    if (entidad->clase() == CLASE_ROCKET) {
        auto *rocket = static_cast<Rocket *>(entidad);
        const short estado = rocket->estadoVisual();
        if (rocket->getVidas() <= 0 && !rocketMuerte.isEmpty())
            lista = &rocketMuerte;
        else if (rocket->esInvulnerable() && rocket->getSalud() < rocket->getSaludMax() && !rocketDanio.isEmpty())
            lista = &rocketDanio;
        else if (estado == 3)
            lista = &impulso;
        else if (rocket->estaFurioso())
            lista = &furia;
        else if (estado == 2)
            lista = &saltar;
        else if (granadaMuestra > 0.f)
            lista = &lanzar;
        else if (teclas.contains(Qt::Key_Space))
            lista = &disparar;
        else if (estado == 0)
            lista = espera.isEmpty() ? &caminar : &espera;
        else
            lista = &caminar;
        if (rocket->getVidas() <= 0)
            indice = lista->isEmpty() ? 0 : lista->size() - 1;
        else if (estado == 0 && lista != &rocketDanio)
            indice = 0;
        else
            indice = int(tiempo * (estado == 1 ? 8.f : 12.f));
    } else if (entidad->clase() == CLASE_ENEMIGO) {
        auto *enemigo = static_cast<Enemigo *>(entidad);
        const short aspecto = enemigo->getAspecto();
        const bool jinete = enemigo->getEsCorreo() || aspecto >= 2;
        if (jinete) {
            if (aspecto == 3 && !jineteBandera.isEmpty())
                lista = &jineteBandera;
            else if (aspecto == 4 && !jineteTrompeta.isEmpty())
                lista = &jineteTrompeta;
            else
                lista = &jineteGalope;
        } else if (aspecto == 1 && !soldadoBandera.isEmpty()) {
            lista = &soldadoBandera;
        } else if (enemigo->estadoVisual() == 1)
            lista = &soldadoDispara;
        else
            lista = &soldadoCamina;
        indice = int(tiempo * 7.f);
    } else if (entidad->clase() == CLASE_ESTRUCTURA) {
        auto *pieza = static_cast<Estructura *>(entidad);
        if (pieza->getTipo() == EST_PUENTE)
            return puenteQuieto;
        if (pieza->getTipo() == EST_BARRIL) {
            lista = pieza->estaDestruida() && !barrilEstalla.isEmpty() ? &barrilEstalla : &barrilQuieto;
            indice = int(tiempo * 8.f);
        } else if (pieza->estaDestruida())
            lista = &carretaExplota;
        else if (pieza->getResistencia() < 40.f)
            lista = &carretaHerida;
        else
            lista = &carretaRueda;
        indice = int(tiempo * 6.f);
    }

    if (!lista || lista->isEmpty())
        return {};
    return lista->at(indice % lista->size());
}

void EscenaEmboscada::colocar(Entidad *entidad, QGraphicsItem *dibujo) {
    auto *sprite = dynamic_cast<QGraphicsPixmapItem *>(dibujo);
    if (!sprite)
        return;

    QPixmap cuadro = cuadroDe(entidad);
    if (cuadro.isNull())
        return;
    if (miraALaIzquierda(entidad))
        cuadro = voltear(cuadro);

    float altoDibujo = entidad->getAlto();
    float fuente = 0.f;
    float fraccionApoyo = 0.55f;
    bool estalla = false;
    if (entidad->clase() == CLASE_ROCKET) {
        altoDibujo = 118.f;
        fuente = caminar.isEmpty() ? 0.f : float(caminar.first().height());
    } else if (entidad->clase() == CLASE_ENEMIGO) {
        auto *enemigo = static_cast<Enemigo *>(entidad);
        const bool jinete = enemigo->getEsCorreo() || enemigo->getAspecto() >= 2;
        altoDibujo = jinete ? 128.f : 108.f;
        const QVector<QPixmap> &ref = jinete ? jineteGalope : soldadoCamina;
        fuente = ref.isEmpty() ? 0.f : float(ref.first().height());
        fraccionApoyo = 0.22f;
    } else if (entidad->clase() == CLASE_ESTRUCTURA) {
        auto *pieza = static_cast<Estructura *>(entidad);
        if (pieza->getTipo() == EST_CARRETA)
            altoDibujo = pieza->estaDestruida() ? 124.f : 112.f;
        else if (pieza->getTipo() == EST_BARRIL) {
            altoDibujo = 72.f;
            estalla = pieza->estaDestruida();
        }
    }

    const bool esPuente = entidad->clase() == CLASE_ESTRUCTURA
        && static_cast<Estructura *>(entidad)->getTipo() == EST_PUENTE;
    if (esPuente) {
        sprite->setPixmap(cuadro.scaledToWidth(qMax(1, qRound(entidad->getAncho())), Qt::SmoothTransformation));
        sprite->setPos(entidad->getCentroX() - sprite->pixmap().width() / 2.f, entidad->getY() - sprite->pixmap().height() * 0.58f);
    } else if (estalla) {
        const QPixmap puesto = cuadro.scaledToHeight(qMax(1, qRound(altoDibujo)), Qt::SmoothTransformation);
        sprite->setPixmap(puesto);
        sprite->setPos(entidad->getCentroX() - puesto.width() / 2.f, entidad->getCentroY() - puesto.height() / 2.f);
    } else {
        apoyar(sprite, cuadro, entidad->getCentroX(), entidad->getY() + entidad->getAlto(), altoDibujo, fuente, fraccionApoyo);
    }
    if (auto *rocket = dynamic_cast<Rocket *>(entidad))
        sprite->setOpacity(!rocket->esInvulnerable() || std::fmod(tiempo, 0.24f) < 0.12f ? 1.0 : 0.35);
    else
        sprite->setOpacity(1.0);
}

void EscenaEmboscada::sincronizar() {
    if (!nivel)
        return;

    const std::vector<Entidad *> &lista = nivel->obtenerEntidades();
    QSet<Entidad *> vivas;

    for (Entidad *entidad : lista) {
        vivas.insert(entidad);
        QGraphicsItem *dibujo = piezas.value(entidad, nullptr);
        if (!dibujo) {
            if (entidad->clase() == CLASE_PROYECTIL) {
                auto *bala = static_cast<Proyectil *>(entidad);
                QColor color = bala->esDeRocket() ? QColor(232, 196, 74) : QColor(176, 42, 36);
                int diametro = bala->esBombeado() ? 18 : (bala->esDeRocket() ? 12 : 14);
                if (bala->esBombeado())
                    color = QColor(86, 122, 62);
                auto *punto = addPixmap(circulo(color, diametro));
                punto->setZValue(4);
                dibujo = punto;
            } else {
                auto *sprite = addPixmap(QPixmap());
                sprite->setZValue(entidad->clase() == CLASE_ESTRUCTURA ? 2 : 3);
                sprite->setData(0, entidad->clase());
                if (entidad->clase() == CLASE_ENEMIGO) {
                    auto *enemigo = static_cast<Enemigo *>(entidad);
                    sprite->setData(1, enemigo->getEsCorreo() || enemigo->getAspecto() >= 2 ? 1 : 0);
                    sprite->setData(2, enemigo->getAspecto());
                }
                dibujo = sprite;
            }
            piezas.insert(entidad, dibujo);
        }

        if (entidad->clase() == CLASE_PROYECTIL)
            dibujo->setPos(entidad->getX(), entidad->getY());
        else
            colocar(entidad, dibujo);
    }

    const auto llaves = piezas.keys();
    for (Entidad *entidad : llaves) {
        if (vivas.contains(entidad))
            continue;
        QGraphicsItem *dibujo = piezas.take(entidad);
        if (dibujo->data(0).toInt() == CLASE_ENEMIGO) {
            auto *sprite = dynamic_cast<QGraphicsPixmapItem *>(dibujo);
            const bool jinete = dibujo->data(1).toInt() != 0;
            const QVector<QPixmap> &anim = jinete ? jineteMuere : soldadoMuere;
            const QVector<QPixmap> &ref = jinete ? jineteGalope : soldadoCamina;
            if (sprite && !anim.isEmpty()) {
                Caida caida;
                caida.item = addPixmap(QPixmap());
                caida.item->setZValue(3);
                caida.cuadros = anim;
                caida.centro = float(sprite->pos().x() + sprite->pixmap().width() / 2.0);
                caida.suelo = float(sprite->pos().y() + sprite->pixmap().height());
                caida.alto = jinete ? 128.f : 108.f;
                caida.fuente = ref.isEmpty() ? 0.f : float(ref.first().height());
                apoyar(caida.item, anim.first(), caida.centro, caida.suelo, caida.alto, caida.fuente, 0.22f);
                caidas.append(caida);
            }
        }
        if (dibujo->zValue() >= 4.f) {
            const QPointF pos = dibujo->pos();
            auto *nube = addEllipse(pos.x() - 7.f, pos.y() - 7.f, 18, 18,
                QPen(Qt::NoPen), QBrush(QColor(244, 228, 176, 210)));
            nube->setZValue(5);
            flashes.append(nube);
            vidaFlash.append(0.12f);
        }
        removeItem(dibujo);
        delete dibujo;
    }

    std::vector<short> eventos;
    nivel->tomarEventos(eventos);
    dibujarHud();
}
