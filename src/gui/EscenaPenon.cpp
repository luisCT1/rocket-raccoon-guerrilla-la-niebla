#include "EscenaPenon.h"

#include "AgenteTipos.h"
#include "Capitan.h"
#include "Enemigo.h"
#include "Estructura.h"
#include "Eventos.h"
#include "Juego.h"
#include "NivelPenon.h"
#include "Proyectil.h"
#include "Recogible.h"
#include "Rocket.h"
#include "Sonido.h"

#include <QFont>
#include <QImage>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QPainter>

#include <algorithm>
#include <cmath>

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

QPixmap balaBrillante(const QColor &color, int diametro, bool furia) {
    const int lado = diametro + 10;
    QPixmap mapa(lado, lado);
    mapa.fill(Qt::transparent);
    QPainter pintor(&mapa);
    pintor.setRenderHint(QPainter::Antialiasing);
    pintor.setPen(Qt::NoPen);
    pintor.setBrush(furia ? QColor(255, 70, 16, 120) : QColor(color.red(), color.green(), color.blue(), 90));
    pintor.drawEllipse(0, 0, lado - 1, lado - 1);
    pintor.setPen(QPen(furia ? QColor(70, 8, 4) : QColor(36, 18, 6), 2));
    pintor.setBrush(color);
    pintor.drawEllipse(5, 5, diametro - 1, diametro - 1);
    pintor.setPen(Qt::NoPen);
    pintor.setBrush(furia ? QColor(255, 214, 120) : QColor(255, 248, 214));
    const int nucleo = qMax(6, diametro / 3);
    pintor.drawEllipse(5 + (diametro - nucleo) / 2, 5 + (diametro - nucleo) / 2, nucleo, nucleo);
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

QVector<QPixmap> cuadros(const QPixmap &hoja, std::initializer_list<QRect> zonas, bool quitarRotulo = false) {
    QVector<QPixmap> salida;
    if (hoja.isNull())
        return salida;
    for (const QRect &zona : zonas) {
        const QRect recorte = zona.intersected(hoja.rect());
        if (recorte.isEmpty())
            continue;
        QImage img = hoja.copy(recorte).toImage().convertToFormat(QImage::Format_ARGB32);
        if (quitarRotulo)
            img = sinRotulo(img);
        const QPixmap cuadro = ajustar(img);
        if (cuadro.isNull() || cuadro.width() < 36 || cuadro.height() < 36)
            continue;
        if (quitarRotulo && cuadro.height() < cuadro.width() * 0.55f)
            continue;
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

int lineaSuperficie(const QPixmap &cuadro) {
    const QImage img = cuadro.toImage();
    const int ancho = img.width();
    const int necesario = qMax(8, int(ancho * 0.5f));
    for (int y = 0; y < img.height(); ++y) {
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
    return 0;
}

QPixmap pieza(const QPixmap &hoja, const QRect &zona) {
    const QVector<QPixmap> lista = cuadros(hoja, {zona});
    return lista.isEmpty() ? QPixmap() : lista.first();
}
}

EscenaPenon::EscenaPenon() {
    setSceneRect(0, 0, 3920, 720);
    setBackgroundBrush(QColor(150, 176, 186));
    sonidos = new Sonido(this);
    juego = new Juego();
    juego->iniciarNivel(NIVEL_PENON, DIF_NORMAL);
    nivel = dynamic_cast<NivelPenon *>(juego->nivelActivo());
    if (nivel)
        setSceneRect(0, 0, nivel->getAnchoMundo(), nivel->getAltoMundo());
    cargarSprites();
    armarMundo();
}

EscenaPenon::~EscenaPenon() {
    delete sonidos;
    delete juego;
}

void EscenaPenon::cargarSprites() {
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
    agachar = cuadros(rocket, {QRect(846, 263, 166, 98)}, false);
    rocketDanio = cuadros(rocket, {QRect(522, 960, 148, 151), QRect(684, 960, 147, 151)}, false);
    rocketMuerte = cuadros(rocket, {QRect(684, 960, 147, 151), QRect(851, 960, 154, 151)}, false);

    const QPixmap capitan(QStringLiteral(":/nivel3/capitan.png"));
    capitanCamina = cuadros(capitan, {QRect(16, 24, 123, 165), QRect(183, 24, 120, 165), QRect(354, 24, 115, 165),
                                       QRect(518, 24, 121, 165), QRect(699, 24, 98, 165), QRect(865, 24, 104, 165)});
    capitanEspera = cuadros(capitan, {QRect(24, 215, 109, 165), QRect(197, 215, 106, 165),
                                       QRect(368, 215, 100, 165), QRect(541, 215, 99, 165)});
    capitanAcosa = cuadros(capitan, {QRect(8, 403, 155, 168), QRect(165, 403, 160, 168), QRect(330, 403, 170, 168),
                                      QRect(505, 403, 170, 168), QRect(675, 403, 165, 168), QRect(845, 403, 170, 168)});
    capitanCanon = cuadros(capitan, {QRect(15, 590, 146, 161), QRect(182, 590, 145, 161), QRect(349, 590, 152, 161),
                                      QRect(524, 590, 124, 161), QRect(687, 590, 154, 161), QRect(855, 590, 122, 161)});
    capitanGiro = cuadros(capitan, {QRect(21, 777, 132, 161), QRect(187, 777, 121, 161), QRect(349, 777, 150, 161),
                                     QRect(515, 777, 157, 161), QRect(684, 777, 148, 161), QRect(851, 777, 159, 161)});
    capitanTromba = cuadros(capitan, {QRect(16, 970, 144, 157), QRect(180, 970, 140, 157), QRect(353, 970, 143, 157),
                                       QRect(523, 970, 137, 157), QRect(693, 970, 140, 157), QRect(851, 970, 152, 157)});
    capitanRepliegue = cuadros(capitan, {QRect(18, 1159, 129, 154), QRect(186, 1159, 136, 154),
                                          QRect(356, 1159, 140, 154), QRect(530, 1159, 133, 154)});
    capitanMuerte = cuadros(capitan, {QRect(15, 1342, 117, 161), QRect(174, 1342, 118, 161), QRect(315, 1342, 115, 161),
                                       QRect(468, 1342, 145, 161), QRect(638, 1342, 178, 161), QRect(830, 1342, 173, 161)});

    const QPixmap batalla(QStringLiteral(":/nivel3/batalla.png"));
    canonCuadros = cuadros(batalla, {QRect(16, 31, 290, 245), QRect(320, 31, 300, 245),
                                      QRect(630, 31, 310, 245), QRect(950, 31, 290, 245)});
    for (QPixmap &cuadro : canonCuadros)
        cuadro = voltear(cuadro);
    balaCanon = cuadros(batalla, {QRect(109, 437, 86, 84)});
    explosion = cuadros(batalla, {QRect(326, 333, 279, 263), QRect(637, 333, 287, 263), QRect(953, 333, 284, 263)});
    banderaCuadros = cuadros(batalla, {QRect(44, 635, 157, 276), QRect(357, 635, 222, 276),
                                        QRect(643, 635, 264, 276), QRect(956, 635, 272, 276)});
    viento = cuadros(batalla, {QRect(24, 945, 246, 277), QRect(356, 945, 225, 277),
                                QRect(658, 945, 244, 277), QRect(973, 945, 250, 277)});
    QImage filo(180, 130, QImage::Format_ARGB32_Premultiplied);
    filo.fill(Qt::transparent);
    QPainter tinta(&filo);
    tinta.setRenderHint(QPainter::Antialiasing, true);
    tinta.setPen(QPen(QColor(255, 228, 150, 230), 8, Qt::SolidLine, Qt::RoundCap));
    tinta.drawArc(QRect(8, 18, 160, 96), 20 * 16, 150 * 16);
    tinta.setPen(QPen(QColor(255, 255, 245, 240), 3, Qt::SolidLine, Qt::RoundCap));
    tinta.drawArc(QRect(14, 26, 148, 80), 28 * 16, 136 * 16);
    tinta.end();
    tajoCuadro = QPixmap::fromImage(filo);

    const QPixmap plataformas(QStringLiteral(":/nivel3/plataformas.png"));
    suelos = cuadros(plataformas, {QRect(26, 116, 262, 181), QRect(338, 116, 262, 181),
                                    QRect(654, 116, 257, 181), QRect(969, 116, 257, 181)});
    losas = cuadros(plataformas, {QRect(18, 386, 280, 217), QRect(357, 386, 230, 217), QRect(637, 386, 284, 217)});
    arco = pieza(plataformas, QRect(328, 898, 283, 284));
    ruina = pieza(plataformas, QRect(647, 898, 276, 284));
    arbolCima = pieza(plataformas, QRect(971, 898, 252, 284));

    const QPixmap vegetacion(QStringLiteral(":/nivel3/vegetacion.png"));
    const QVector<QPixmap> selva = cuadros(vegetacion, {
        QRect(9, 13, 373, 486), QRect(422, 13, 340, 486), QRect(774, 13, 367, 486), QRect(1182, 13, 336, 486),
        QRect(15, 526, 394, 427), QRect(426, 526, 351, 427), QRect(785, 526, 344, 427), QRect(1156, 526, 365, 427)});
    for (int i = 0; i < selva.size(); ++i) {
        if (i < 4)
            arboles.append(selva.at(i));
        else if (i < 6)
            helechos.append(selva.at(i));
        else
            rocas.append(selva.at(i));
    }

    prepararHud(QPixmap(QStringLiteral(":/nivel1/sprint_vidas_municion.png")));

    const QPixmap hojaIconos(QStringLiteral(":/nivel1/sprint_iconos.png"));
    iconos = {
        hojaIconos.copy(200, 41, 735, 426),
        hojaIconos.copy(189, 534, 738, 456),
        hojaIconos.copy(219, 1005, 624, 470),
        hojaIconos.copy(203, 1485, 660, 562)
    };
}

void EscenaPenon::prepararHud(const QPixmap &hoja) {
    if (!hoja.isNull())
        prepararMarcador(hoja.toImage().convertToFormat(QImage::Format_ARGB32), piezasHud);
    return;
    if (hoja.isNull())
        return;
    const QImage src = hoja.toImage().convertToFormat(QImage::Format_ARGB32);
    const QColor tinta = src.rect().contains(QPoint(560, 160)) ? src.pixelColor(560, 160) : QColor(18, 32, 58);

    panelVidas = src.copy(32, 5, 980, 369);
    const QRect absolutos[3] = {QRect(485, 182, 129, 109), QRect(638, 182, 128, 109), QRect(790, 182, 129, 109)};
    for (const QRect &abs : absolutos) {
        const QRect local = abs.translated(-32, -5);
        if (!panelVidas.rect().contains(local))
            continue;
        corazones.append(panelVidas.copy(local));
        posCorazon.append(local.topLeft());
        for (int y = local.top(); y <= local.bottom(); ++y) {
            for (int x = local.left(); x <= local.right(); ++x)
                panelVidas.setPixelColor(x, y, tinta);
        }
    }

    barraLlena = ajustar(src.copy(1082, 5, 1065, 369)).toImage();
    barraVacia = barraLlena;
    if (!barraLlena.isNull()) {
        int rx0 = barraLlena.width();
        int ry0 = barraLlena.height();
        int rx1 = -1;
        int ry1 = -1;
        int tx0 = barraLlena.width();
        int ty0 = barraLlena.height();
        int tx1 = -1;
        int ty1 = -1;
        for (int y = 0; y < barraLlena.height(); ++y) {
            for (int x = 0; x < barraLlena.width(); ++x) {
                const QRgb p = barraLlena.pixel(x, y);
                if (qAlpha(p) > 200 && qRed(p) > 170 && qGreen(p) < 80 && qBlue(p) < 90) {
                    rx0 = std::min(rx0, x);
                    ry0 = std::min(ry0, y);
                    rx1 = std::max(rx1, x);
                    ry1 = std::max(ry1, y);
                }
                if (x > barraLlena.width() / 2 && qAlpha(p) > 200 && qRed(p) > 220 && qGreen(p) > 210 && qBlue(p) > 190) {
                    tx0 = std::min(tx0, x);
                    ty0 = std::min(ty0, y);
                    tx1 = std::max(tx1, x);
                    ty1 = std::max(ty1, y);
                }
            }
        }
        if (rx1 > rx0)
            rojoBarra = QRect(rx0, ry0, rx1 - rx0 + 1, ry1 - ry0 + 1);
        if (tx1 > tx0)
            textoBarra = QRect(tx0, ty0, tx1 - tx0 + 1, ty1 - ty0 + 1);
        const QColor pista = ry0 > 2 ? barraLlena.pixelColor(qBound(0, rx0 + 6, barraLlena.width() - 1), qMax(0, ry0 - 2)) : tinta;
        const QColor placa = tx1 > 0 ? barraLlena.pixelColor(qMin(barraLlena.width() - 1, tx1 + 3), (ty0 + ty1) / 2) : tinta;
        for (int y = rojoBarra.top(); y <= rojoBarra.bottom(); ++y) {
            for (int x = rojoBarra.left(); x <= rojoBarra.right(); ++x)
                barraVacia.setPixelColor(x, y, pista);
        }
        for (int y = textoBarra.top(); y <= textoBarra.bottom(); ++y) {
            for (int x = textoBarra.left(); x <= textoBarra.right(); ++x)
                barraVacia.setPixelColor(x, y, placa);
        }
    }

    const QImage tira = src.copy(32, 418, 1029, 267);
    balasLlenas = ajustar(tira).toImage();
    balasVacias = balasLlenas;
    if (balasLlenas.isNull())
        return;
    const QColor fondo = balasLlenas.pixelColor(balasLlenas.width() / 2, balasLlenas.height() / 2);
    QVector<int> visto(balasLlenas.width() * balasLlenas.height(), 0);
    QVector<int> cola;
    int marca = 0;
    auto oro = [](QRgb p) {
        return qAlpha(p) > 200 && qRed(p) > 190 && qGreen(p) > 140 && qBlue(p) < 110;
    };
    for (int y = 0; y < balasLlenas.height(); ++y) {
        for (int x = balasLlenas.width() / 4; x < balasLlenas.width() * 2 / 3; ++x) {
            const int origen = y * balasLlenas.width() + x;
            if (visto.at(origen) || !oro(balasLlenas.pixel(x, y)))
                continue;
            ++marca;
            int x0 = x;
            int y0 = y;
            int x1 = x;
            int y1 = y;
            cola.clear();
            cola.append(origen);
            visto[origen] = marca;
            for (int k = 0; k < cola.size(); ++k) {
                const int i = cola.at(k);
                const int px = i % balasLlenas.width();
                const int py = i / balasLlenas.width();
                x0 = std::min(x0, px);
                y0 = std::min(y0, py);
                x1 = std::max(x1, px);
                y1 = std::max(y1, py);
                const int dx[4] = {-1, 1, 0, 0};
                const int dy[4] = {0, 0, -1, 1};
                for (int v = 0; v < 4; ++v) {
                    const int nx = px + dx[v];
                    const int ny = py + dy[v];
                    if (nx < 0 || ny < 0 || nx >= balasLlenas.width() || ny >= balasLlenas.height())
                        continue;
                    const int ni = ny * balasLlenas.width() + nx;
                    if (visto.at(ni) || !oro(balasLlenas.pixel(nx, ny)))
                        continue;
                    visto[ni] = marca;
                    cola.append(ni);
                }
            }
            if (y1 - y0 < 18 || (y1 - y0) < (x1 - x0))
                continue;
            iconosBala.append(balasLlenas.copy(x0, y0, x1 - x0 + 1, y1 - y0 + 1));
            posBala.append(QPoint(x0, y0));
            for (int py = y0; py <= y1; ++py) {
                for (int px = x0; px <= x1; ++px)
                    balasVacias.setPixelColor(px, py, fondo);
            }
        }
    }
    int tx0 = balasLlenas.width();
    int ty0 = balasLlenas.height();
    int tx1 = -1;
    int ty1 = -1;
    for (int y = 0; y < balasLlenas.height(); ++y) {
        for (int x = balasLlenas.width() * 3 / 5; x < balasLlenas.width(); ++x) {
            const QRgb p = balasLlenas.pixel(x, y);
            if (qAlpha(p) > 200 && qRed(p) > 220 && qGreen(p) > 210 && qBlue(p) > 190) {
                tx0 = std::min(tx0, x);
                ty0 = std::min(ty0, y);
                tx1 = std::max(tx1, x);
                ty1 = std::max(ty1, y);
            }
        }
    }
    if (tx1 > tx0) {
        textoBalas = QRect(tx0, ty0, tx1 - tx0 + 1, ty1 - ty0 + 1);
        const QColor placa = balasLlenas.pixelColor(qMax(0, tx0 - 4), (ty0 + ty1) / 2);
        for (int y = textoBalas.top(); y <= textoBalas.bottom(); ++y) {
            for (int x = textoBalas.left(); x <= textoBalas.right(); ++x)
                balasVacias.setPixelColor(x, y, placa);
        }
    }
}

void EscenaPenon::armarMundo() {
    if (!nivel)
        return;
    const float piso = nivel->getSuelo();
    const float hueco0 = nivel->getHuecoInicio();
    const float hueco1 = nivel->getHuecoFin();
    const float ancho = nivel->getAnchoMundo();
    const float cima = nivel->getBanderaY() + 32.f;

    auto ponerCapa = [this](const QPixmap &imagen, float x, float y, qreal z, float avance,
                            float deriva = 0.f, float ciclo = 0.f, float opacidad = 1.f) {
        if (imagen.isNull())
            return;
        auto *trozo = addPixmap(imagen);
        trozo->setPos(x, y);
        trozo->setZValue(z);
        trozo->setOpacity(opacidad);
        capas.append({trozo, x, y, avance, deriva, ciclo});
    };

    const QPixmap cielo(QStringLiteral(":/nivel1/cielo.png"));
    if (!cielo.isNull()) {
        setBackgroundBrush(cielo.toImage().pixelColor(160, 40));
        QPixmap lejos = cielo.scaledToHeight(720, Qt::SmoothTransformation);
        if (lejos.width() < 1700)
            lejos = cielo.scaled(1700, 720, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        ponerCapa(lejos, -180.f, 0.f, 0.05, 0.08);
    }

    const QPixmap niebla(QStringLiteral(":/nivel1/niebla.png"));
    if (!niebla.isNull()) {
        const QPixmap banda = ajustar(niebla.toImage().convertToFormat(QImage::Format_ARGB32)).scaledToWidth(1680, Qt::SmoothTransformation);
        if (!banda.isNull()) {
            for (float x = -float(banda.width()); x <= float(banda.width()); x += banda.width())
                ponerCapa(banda, x, 300.f, 0.12, 0.f, 28.f, float(banda.width()), 0.9);
        }
    }

    const QPixmap camino(QStringLiteral(":/nivel1/camino.png"));
    const QPixmap abismo(QStringLiteral(":/nivel1/avismo.png"));
    const float filaCamino = 316.f;
    const float filaAbismo = 324.f;
    const float baseCamino = 508.f;
    const float escala = (712.f - piso) / (baseCamino - filaCamino);
    const float yCamino = piso - filaCamino * escala;
    const float yAbismo = piso - filaAbismo * escala;
    auto escalar = [escala](const QPixmap &pieza) {
        return pieza.scaled(qMax(1, qRound(pieza.width() * escala)),
                            qMax(1, qRound(pieza.height() * escala)),
                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    };

    if (!abismo.isNull() && !camino.isNull()) {
        const int corte = 1320;
        const int labio = 1514;
        const QPixmap borde = escalar(abismo.copy(corte, 0, abismo.width() - corte, abismo.height()));
        const QPixmap vuelto = borde.transformed(QTransform().scale(-1, 1), Qt::FastTransformation);
        const float labioLocal = (labio - corte) * escala;
        const float xIzq = hueco0 - labioLocal;
        const float xDer = hueco1 - (borde.width() - labioLocal);
        const int largoIzq = qMin(camino.width(), qRound((xIzq + 40.f) / escala));
        ponerCapa(escalar(camino.copy(0, 0, largoIzq, camino.height())), 0.f, yCamino, 0.4, 1.f);
        ponerCapa(borde, xIzq, yAbismo, 0.46, 1.f);
        ponerCapa(vuelto, xDer, yAbismo, 0.46, 1.f);
        float x = xDer + borde.width() - 46.f;
        const QPixmap tramo = escalar(camino);
        while (x < ancho + 20.f) {
            ponerCapa(tramo, x, yCamino, 0.4, 1.f);
            x += tramo.width() - 36.f;
        }
    }

    auto plantar = [&](const QPixmap &imagen, float x, float base, float alto, qreal z) {
        if (imagen.isNull())
            return;
        auto *trozo = addPixmap(QPixmap());
        apoyar(trozo, imagen, x, base, alto);
        trozo->setZValue(z);
    };

    const float puestos[] = { 80.f, 430.f, 820.f, 1180.f, 1560.f, 2860.f, 3320.f, 3680.f };
    for (int i = 0; i < 8 && !arboles.isEmpty(); ++i)
        plantar(arboles.at(i % arboles.size()), puestos[i], piso + 2.f, i < 5 ? 300.f : 250.f, 1.15);

    if (!arco.isNull())
        plantar(arco, hueco0 - 220.f, piso + 2.f, 250.f, 1.3);
    if (nivel->getCapitan() && !ruina.isNull())
        plantar(ruina, nivel->getCapitan()->getX() + 180.f, piso + 2.f, 280.f, 1.2);
    if (!arbolCima.isNull())
        plantar(arbolCima, nivel->getBanderaX() - 10.f, cima + 4.f, 190.f, 2.05);

    for (int i = 0; i < rocas.size(); ++i) {
        const float x = i == 0 ? 560.f : 1280.f;
        plantar(rocas.at(i), x, piso + 2.f, i == 0 ? 120.f : 78.f, 1.35);
    }

    const float helechosX[] = { 260.f, 980.f, 1380.f, 3600.f };
    for (int i = 0; i < 4 && !helechos.isEmpty(); ++i)
        plantar(helechos.at(i % helechos.size()), helechosX[i], piso + 2.f, 110.f, 1.5);

    for (int i = 0; i < arboles.size(); ++i) {
        const QPixmap lejos = arboles.at(i).scaledToHeight(170, Qt::SmoothTransformation);
        ponerCapa(lejos, 40.f + i * 520.f, piso - lejos.height() + 36.f, 0.18, 0.38, 0.f, 0.f, 0.75);
    }
}

void EscenaPenon::moverCapas() {
    const float cam = nivel ? nivel->getCamaraX() : 0.f;
    for (const Capa &capa : capas) {
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

void EscenaPenon::pulsar(int tecla) {
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

void EscenaPenon::soltar(int tecla) {
    teclas.remove(tecla);
}

void EscenaPenon::mantener(int tecla) {
    teclas.insert(tecla);
}

void EscenaPenon::apuntar(float, float) {}

void EscenaPenon::avanzar(float dt) {
    if (!nivel)
        return;
    tiempo += dt;
    if (granadaMuestra > 0.f)
        granadaMuestra = qMax(0.f, granadaMuestra - dt);
    if (canonPulso > 0.f)
        canonPulso = qMax(0.f, canonPulso - dt);
    if (avisoTiempo > 0.f)
        avisoTiempo = qMax(0.f, avisoTiempo - dt);
    if (nivel->getCapitan() && nivel->getCapitan()->estaDerrotado())
        edadMuerte += dt;

    Rocket *rocket = nivel->jugador();
    if (rocket && !nivel->estaTerminado()) {
        if (teclas.contains(Qt::Key_A) || teclas.contains(Qt::Key_Left))
            nivel->jugadorMueve('L');
        if (teclas.contains(Qt::Key_D) || teclas.contains(Qt::Key_Right))
            nivel->jugadorMueve('R');
        rocket->fijarAgachado(teclas.contains(Qt::Key_S) || teclas.contains(Qt::Key_Down));
        const float dir = rocket->getDireccion() == 'L' ? -1.f : 1.f;
        if (teclas.contains(Qt::Key_Space))
            nivel->jugadorDispara(rocket->getCentroX() + dir * 480.f, rocket->getY() + rocket->getAlto() * 0.35f);
    }

    QHash<Entidad *, QPointF> canonAntes;
    for (Entidad *entidad : nivel->obtenerEntidades()) {
        if (entidad->clase() != CLASE_PROYECTIL)
            continue;
        auto *bala = static_cast<Proyectil *>(entidad);
        if (!bala->esDeRocket() && bala->esBombeado())
            canonAntes.insert(entidad, QPointF(bala->getCentroX(), bala->getCentroY()));
    }

    juego->actualizar(dt);
    moverCapas();
    sincronizar();

    const std::vector<Entidad *> &vivas = nivel->obtenerEntidades();
    QSet<Entidad *> siguen;
    for (Entidad *entidad : vivas)
        siguen.insert(entidad);
    for (auto marca = canonAntes.cbegin(); marca != canonAntes.cend(); ++marca) {
        if (siguen.contains(marca.key()))
            continue;
        Estallido estallido;
        estallido.item = addPixmap(QPixmap());
        estallido.item->setZValue(5);
        estallido.x = float(marca.value().x());
        estallido.y = float(marca.value().y());
        estallidos.append(estallido);
    }

    for (int i = estallidos.size() - 1; i >= 0; --i) {
        Estallido &estallido = estallidos[i];
        estallido.edad += dt;
        const bool fuera = explosion.isEmpty() || estallido.edad > explosion.size() * 0.09f;
        if (fuera) {
            removeItem(estallido.item);
            delete estallido.item;
            estallidos.removeAt(i);
            continue;
        }
        const QPixmap cuadro = explosion.at(qMin(int(estallido.edad * 11.f), explosion.size() - 1));
        const QPixmap puesto = cuadro.scaledToHeight(150, Qt::SmoothTransformation);
        estallido.item->setPixmap(puesto);
        estallido.item->setPos(estallido.x - puesto.width() / 2.f, estallido.y - puesto.height() / 2.f);
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
}

QRectF EscenaPenon::rectCamara() const {
    const float x = nivel ? nivel->getCamaraX() : 0.f;
    return QRectF(x, 0, kVistaAncho, kVistaAlto);
}

QString EscenaPenon::textoHud() const {
    return {};
}

QString EscenaPenon::mensajeFinal() const {
    if (!nivel || !nivel->estaTerminado() || !nivel->jugador())
        return {};
    if (nivel->fueGanado())
        return QStringLiteral("Misión cumplida\nLa bandera patriota quedó en el Peñón.");
    if (nivel->jugador()->cayoAlVacio())
        return QStringLiteral("Misión fallida\nCaíste al vacío y te quedaste sin oportunidades.");
    return QStringLiteral("Misión fallida\nEl capitán te derrotó.");
}

bool EscenaPenon::acabo() const {
    return nivel && nivel->estaTerminado();
}

bool EscenaPenon::enFuria() const {
    Rocket *rocket = nivel ? nivel->jugador() : nullptr;
    return rocket && rocket->estaFurioso();
}

bool EscenaPenon::gano() const {
    return nivel && nivel->fueGanado();
}

bool EscenaPenon::miraALaIzquierda(Entidad *entidad) const {
    if (auto *rocket = dynamic_cast<Rocket *>(entidad))
        return rocket->getDireccion() == 'L';
    if (auto *capitan = dynamic_cast<Capitan *>(entidad))
        return capitan->getDireccion() == 'L';
    return false;
}

QPixmap EscenaPenon::cuadroDe(Entidad *entidad) const {
    const QVector<QPixmap> *lista = nullptr;
    int indice = 0;

    if (entidad->clase() == CLASE_RECOGIBLE) {
        const short tipo = static_cast<Recogible *>(entidad)->getTipo();
        if (tipo >= 0 && tipo < iconos.size())
            return iconos.at(tipo);
        return {};
    }

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
        else if (rocket->estaAgachado() && !agachar.isEmpty())
            lista = &agachar;
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
    } else if (entidad->clase() == CLASE_CAPITAN) {
        auto *capitan = static_cast<Capitan *>(entidad);
        const short estado = capitan->estadoVisual();
        const bool camina = std::abs(capitan->getX() - capitanX) > 0.5f;
        if (estado == 99)
            lista = &capitanMuerte;
        else if (estado == 1)
            lista = &capitanAcosa;
        else if (estado == 2)
            lista = &capitanCanon;
        else if (estado == 3)
            lista = &capitanGiro;
        else if (estado == 4)
            lista = &capitanTromba;
        else if (estado == 5)
            lista = &capitanRepliegue;
        else if (camina)
            lista = &capitanCamina;
        else
            lista = &capitanEspera;
        if (estado == 99)
            indice = lista->isEmpty() ? 0 : qMin(int(edadMuerte * 8.f), lista->size() - 1);
        else if (estado == 0 && !camina)
            indice = 0;
        else
            indice = int(tiempo * 8.f);
    } else if (entidad->clase() == CLASE_ESTRUCTURA) {
        auto *piezaE = static_cast<Estructura *>(entidad);
        if (piezaE->getTipo() == EST_CANON) {
            lista = &canonCuadros;
            if (piezaE->estaDestruida())
                indice = lista->isEmpty() ? 0 : lista->size() - 1;
            else if (canonPulso > 0.18f)
                indice = qMin(2, qMax(0, lista->size() - 1));
            else if (canonPulso > 0.f)
                indice = qMin(1, qMax(0, lista->size() - 1));
        } else if (piezaE->getTipo() == EST_PLATAFORMA && !losas.isEmpty()) {
            int cual = 0;
            if (entidad->getX() > 2400.f)
                cual = 2;
            else if (entidad->getX() > 2200.f)
                cual = 1;
            return losas.at(cual % losas.size());
        }
    }

    if (!lista || lista->isEmpty())
        return {};
    if (indice < 0)
        indice = 0;
    return lista->at(indice % lista->size());
}

void EscenaPenon::apoyar(QGraphicsPixmapItem *sprite, const QPixmap &cuadro, float centro, float suelo, float alto, float altoFuente, float fraccion) {
    if (!sprite || cuadro.isNull())
        return;
    const float ref = altoFuente > 1.f ? altoFuente : float(qMax(1, cuadro.height()));
    const float factor = alto / ref;
    const QPixmap puesto = cuadro.scaledToHeight(qMax(1, qRound(cuadro.height() * factor)), Qt::SmoothTransformation);
    sprite->setPixmap(puesto);
    sprite->setPos(centro - puesto.width() / 2.f, suelo - float(lineaApoyo(cuadro, fraccion) + 1) * factor);
}

void EscenaPenon::apoyarSuperficie(QGraphicsPixmapItem *sprite, const QPixmap &cuadro, float izquierda, float superficie, float ancho) {
    if (!sprite || cuadro.isNull())
        return;
    const QPixmap puesto = cuadro.scaledToWidth(qMax(1, qRound(ancho)), Qt::SmoothTransformation);
    sprite->setPixmap(puesto);
    const float factor = float(puesto.height()) / float(qMax(1, cuadro.height()));
    sprite->setPos(izquierda, superficie - float(lineaSuperficie(cuadro)) * factor);
}

void EscenaPenon::sincronizar() {
    if (!nivel)
        return;

    std::vector<short> eventos;
    nivel->tomarEventos(eventos);
    Capitan *capitan = nivel->getCapitan();
    if (capitan) {
        const short fase = capitan->getFase();
        if (fase != faseVista) {
            faseVista = fase;
            if (fase == FASE_CANON)
                avisoFase = QStringLiteral("El capitán llama al cañón.");
            else if (fase == FASE_AGRESIVO)
                avisoFase = QStringLiteral("El capitán suelta el sable.");
            avisoTiempo = 3.4f;
        }
    }
    for (short evento : eventos) {
        if (evento == EV_CANON_DISPARA)
            canonPulso = 0.42f;
        else if (evento == EV_CAMBIO_FASE_JEFE && avisoTiempo < 0.2f)
            avisoTiempo = 3.4f;
    }
    if (sonidos) {
        Rocket *rocket = nivel->jugador();
        sonidos->eventos(eventos, rocket && rocket->estaFurioso());
    }

    const std::vector<Entidad *> &lista = nivel->obtenerEntidades();
    QSet<Entidad *> vivas;
    for (Entidad *entidad : lista) {
        vivas.insert(entidad);
        QGraphicsItem *dibujo = piezas.value(entidad, nullptr);
        if (!dibujo) {
            if (entidad->clase() == CLASE_PROYECTIL) {
                auto *bala = static_cast<Proyectil *>(entidad);
                QPixmap icono;
                if (!bala->esDeRocket() && bala->esBombeado()) {
                    if (!balaCanon.isEmpty())
                        icono = balaCanon.first().scaledToHeight(46, Qt::SmoothTransformation);
                    if (icono.isNull())
                        icono = balaBrillante(QColor(88, 78, 68), 40, false);
                }
                else if (bala->esDeRocket() && bala->esBombeado())
                    icono = circulo(QColor(86, 122, 62), 18);
                else if (bala->esDeRocket() && bala->getDanio() > 30.f)
                    icono = balaBrillante(QColor(255, 64, 18), 26, true);
                else if (bala->esDeRocket())
                    icono = balaBrillante(QColor(255, 214, 70), 22, false);
                else
                    icono = balaBrillante(QColor(255, 48, 36), 18, false);
                auto *punto = addPixmap(icono);
                punto->setZValue(4);
                dibujo = punto;
            } else {
                auto *sprite = addPixmap(QPixmap());
                sprite->setZValue(entidad->clase() == CLASE_ESTRUCTURA ? 2.2 : 3.2);
                if (entidad->clase() == CLASE_CAPITAN)
                    sprite->setZValue(3.3);
                dibujo = sprite;
            }
            piezas.insert(entidad, dibujo);
        }

        if (entidad->clase() == CLASE_PROYECTIL) {
            if (auto *punto = dynamic_cast<QGraphicsPixmapItem *>(dibujo)) {
                const QSize s = punto->pixmap().size();
                dibujo->setPos(entidad->getCentroX() - s.width() / 2.0, entidad->getCentroY() - s.height() / 2.0);
            } else {
                dibujo->setPos(entidad->getX(), entidad->getY());
            }
            auto *bala = static_cast<Proyectil *>(entidad);
            if (!bala->esDeRocket() && bala->esBombeado()) {
                auto *aviso = avisos.value(entidad, nullptr);
                if (!aviso) {
                    aviso = addEllipse(-48, -12, 96, 24, QPen(QColor(255, 140, 48), 3), QBrush(QColor(255, 90, 24, 150)));
                    aviso->setZValue(3.6);
                    avisos.insert(entidad, aviso);
                }
                aviso->setPos(bala->getDestinoX(), nivel->getSuelo() - 4.f);
            }
        } else if (auto *sprite = dynamic_cast<QGraphicsPixmapItem *>(dibujo)) {
            QPixmap cuadro = cuadroDe(entidad);
            if (cuadro.isNull())
                continue;
            if (miraALaIzquierda(entidad))
                cuadro = voltear(cuadro);
            if (entidad->clase() == CLASE_RECOGIBLE) {
                auto *reco = static_cast<Recogible *>(entidad);
                if (!reco->estaDisponible()) {
                    sprite->setVisible(false);
                    continue;
                }
                sprite->setVisible(true);
                const QPixmap puesto = cuadro.scaledToHeight(58, Qt::SmoothTransformation);
                const float bob = std::sin(tiempo * 3.2f + entidad->getX() * 0.015f) * 6.f;
                sprite->setPixmap(puesto);
                sprite->setPos(entidad->getCentroX() - puesto.width() / 2.f,
                               entidad->getY() + entidad->getAlto() - puesto.height() + bob);
                sprite->setZValue(2.8);
                continue;
            }
            if (entidad->clase() == CLASE_ESTRUCTURA && static_cast<Estructura *>(entidad)->getTipo() == EST_PLATAFORMA) {
                apoyarSuperficie(sprite, cuadro, entidad->getX() - 14.f, entidad->getY(), entidad->getAncho() + 28.f);
            } else {
                float alto = entidad->getAlto();
                float fuente = 0.f;
                float fraccion = 0.55f;
                if (entidad->clase() == CLASE_ROCKET) {
                    alto = 118.f;
                    fuente = caminar.isEmpty() ? 0.f : float(caminar.first().height());
                } else if (entidad->clase() == CLASE_CAPITAN) {
                    alto = 150.f;
                    fuente = capitanCamina.isEmpty() ? 0.f : float(capitanCamina.first().height());
                    fraccion = 0.28f;
                } else if (entidad->clase() == CLASE_ESTRUCTURA) {
                    alto = 112.f;
                }
                apoyar(sprite, cuadro, entidad->getCentroX(), entidad->getY() + entidad->getAlto(), alto, fuente, fraccion);
            }
            if (auto *rocket = dynamic_cast<Rocket *>(entidad))
                sprite->setOpacity(!rocket->esInvulnerable() || std::fmod(tiempo, 0.24f) < 0.12f ? 1.0 : 0.35);
            else
                sprite->setOpacity(1.0);
        }
    }

    const auto llaves = piezas.keys();
    for (Entidad *entidad : llaves) {
        if (vivas.contains(entidad))
            continue;
        QGraphicsItem *dibujo = piezas.take(entidad);
        if (dibujo->zValue() >= 4.f) {
            const QPointF pos = dibujo->pos();
            auto *nube = addEllipse(pos.x() - 6.f, pos.y() - 6.f, 16, 16, QPen(Qt::NoPen), QBrush(QColor(244, 228, 176, 210)));
            nube->setZValue(5);
            flashes.append(nube);
            vidaFlash.append(0.12f);
        }
        removeItem(dibujo);
        delete dibujo;
    }
    const auto avisosViejos = avisos.keys();
    for (Entidad *entidad : avisosViejos) {
        if (vivas.contains(entidad))
            continue;
        QGraphicsEllipseItem *aviso = avisos.take(entidad);
        removeItem(aviso);
        delete aviso;
    }

    if (!bandera) {
        bandera = addPixmap(QPixmap());
        bandera->setZValue(2.4);
    }
    if (!banderaCuadros.isEmpty()) {
        int frame = 0;
        if (nivel->estaBanderaPuesta())
            frame = banderaCuadros.size() - 1;
        else if (capitan && capitan->estaDerrotado())
            frame = 1 + (int(tiempo * 6.f) % qMax(1, banderaCuadros.size() - 1));
        apoyar(bandera, banderaCuadros.at(qBound(0, frame, banderaCuadros.size() - 1)),
               nivel->getBanderaX(), nivel->getBanderaY() + 32.f, 150.f);
    }

    if (!meta) {
        meta = addEllipse(-48, -9, 96, 18, QPen(QColor(226, 177, 90), 2), QBrush(QColor(226, 177, 90, 45)));
        meta->setZValue(2.16);
    }
    const bool mostrarMeta = capitan && capitan->estaDerrotado() && !nivel->estaBanderaPuesta();
    meta->setVisible(mostrarMeta);
    if (mostrarMeta) {
        meta->setPos(nivel->getBanderaX(), nivel->getBanderaY() + 24.f);
        meta->setOpacity(0.4 + 0.45 * (0.5 + 0.5 * std::sin(tiempo * 5.f)));
    }

    if (!rafaga) {
        rafaga = addPixmap(QPixmap());
        rafaga->setZValue(3.5);
        rafaga->hide();
    }
    if (capitan && capitan->trombaActiva() && !viento.isEmpty()) {
        const int frame = int(tiempo * 12.f) % viento.size();
        QPixmap cuadro = viento.at(frame);
        const float dir = capitan->getDireccion() == 'L' ? -1.f : 1.f;
        if (dir < 0.f)
            cuadro = voltear(cuadro);
        const float ancho = qBound(90.f, capitan->radioTromba() * 1.3f, 280.f);
        const QPixmap puesto = cuadro.scaledToWidth(qRound(ancho), Qt::SmoothTransformation);
        rafaga->setPixmap(puesto);
        rafaga->setPos(capitan->getCentroX() + dir * puesto.width() * 0.15f - puesto.width() / 2.f,
                       capitan->getY() + capitan->getAlto() - puesto.height());
        rafaga->show();
    } else {
        rafaga->hide();
    }

    if (!tajo) {
        tajo = addPixmap(tajoCuadro);
        tajo->setZValue(3.7);
        tajo->hide();
    }
    if (capitan && capitan->muestraSable() && !tajoCuadro.isNull()) {
        QPixmap cuadro = tajoCuadro;
        float cx = capitan->getCentroX();
        float cy = capitan->getCentroY() - 18.f;
        if (capitan->giratorioActivo()) {
            capitan->posicionGiratorio(cx, cy);
        } else {
            const float dir = capitan->getDireccion() == 'L' ? -1.f : 1.f;
            cx += dir * 46.f;
            if (dir < 0.f)
                cuadro = voltear(cuadro);
        }
        tajo->setPixmap(cuadro);
        tajo->setPos(cx - cuadro.width() / 2.f, cy - cuadro.height() / 2.f);
        tajo->show();
    } else {
        tajo->hide();
    }

    if (capitan)
        capitanX = capitan->getX();
    dibujarHud();
}

void EscenaPenon::dibujarHud() {
    if (!nivel || !nivel->jugador())
        return;
    Rocket *rocket = nivel->jugador();
    const float camara = nivel->getCamaraX();
    if (!piezasHud.vidas.isNull()) {
        const QImage lienzo = componerMarcador(piezasHud, rocket->getVidas(),
            rocket->getSaludMax() > 0.f ? rocket->getSalud() / rocket->getSaludMax() : 0.f,
            rocket->getMunicion(), rocket->getGranadas(), rocket->getBarraFuria());
        if (!lienzo.isNull()) {
            if (!marcador) {
                marcador = addPixmap(QPixmap());
                marcador->setZValue(40);
            }
            marcador->setPixmap(QPixmap::fromImage(lienzo));
            marcador->setPos(camara + 52.f, 42.f);
        }
    }

    Capitan *capitan = nivel->getCapitan();
    if (!capitan)
        return;
    const float vida = capitan->estaDerrotado() ? 0.f : capitan->getVida();
    const float vidaMax = capitan->getVidaMax() > 0.f ? capitan->getVidaMax() : 1.f;
    const float fraccion = qBound(0.f, vida / vidaMax, 1.f);
    QImage jefe(280, 46, QImage::Format_ARGB32);
    jefe.fill(Qt::transparent);
    QPainter pj(&jefe);
    pj.setPen(QPen(QColor(199, 161, 90), 2));
    pj.setBrush(QColor(22, 32, 24, 220));
    pj.drawRoundedRect(1, 1, 278, 44, 6, 6);
    pj.setPen(Qt::NoPen);
    pj.setBrush(QColor(70, 28, 24));
    pj.drawRoundedRect(12, 26, 256, 12, 3, 3);
    pj.setBrush(fraccion > 0.66f ? QColor(196, 64, 48) : (fraccion > 0.33f ? QColor(214, 140, 48) : QColor(232, 196, 74)));
    pj.drawRoundedRect(12, 26, qRound(256.f * fraccion), 12, 3, 3);
    pj.setPen(QColor(246, 241, 228));
    pj.setFont(QFont(QStringLiteral("Georgia"), 10, QFont::Bold));
    const QString nombre = capitan->estaDerrotado()
                               ? QStringLiteral("Capitán derrotado")
                               : QStringLiteral("Capitán  %1%").arg(qRound(fraccion * 100.f));
    pj.drawText(QRect(12, 4, 256, 20), Qt::AlignLeft | Qt::AlignVCenter, nombre);
    pj.end();
    if (!marcadorJefe) {
        marcadorJefe = addPixmap(QPixmap());
        marcadorJefe->setZValue(40);
    }
    marcadorJefe->setPixmap(QPixmap::fromImage(jefe));
    marcadorJefe->setPos(camara + kVistaAncho - jefe.width() - 52.f, 42.f);
}
