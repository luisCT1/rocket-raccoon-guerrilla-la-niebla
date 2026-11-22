#include "Marcador.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>

namespace {
void pintarCifra(QPainter &pintor, const QRect &caja, const QString &texto, int tamForzado) {
    if (caja.isEmpty())
        return;
    int tam = tamForzado > 0 ? tamForzado : qBound(9, qRound(caja.height() * 0.52f), 18);
    QFont letra(QStringLiteral("Georgia"), tam, QFont::Bold);
    letra.setStyleStrategy(QFont::PreferAntialias);
    letra.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    QFontMetrics medida(letra);
    while (tam > 8 && medida.horizontalAdvance(texto) > caja.width() - 8) {
        --tam;
        letra.setPointSize(tam);
        medida = QFontMetrics(letra);
    }
    pintor.setFont(letra);
    for (int dy = -2; dy <= 2; ++dy) {
        for (int dx = -2; dx <= 2; ++dx) {
            if (dx == 0 && dy == 0)
                continue;
            pintor.setPen(QColor(32, 16, 8));
            pintor.drawText(caja.translated(dx, dy), Qt::AlignCenter, texto);
        }
    }
    pintor.setPen(QColor(255, 228, 150));
    pintor.drawText(caja, Qt::AlignCenter, texto);
}

QRect escalar(const QRect &r, float f) {
    return QRect(qRound(r.x() * f), qRound(r.y() * f), qRound(r.width() * f), qRound(r.height() * f));
}
}

void prepararMarcador(const QImage &hoja, PiezasHud &p) {
    p = PiezasHud();
    if (hoja.isNull())
        return;
    const QImage src = hoja.convertToFormat(QImage::Format_ARGB32);
    p.vidas = src.copy(42, 22, 1227, 456);
    p.salud = src.copy(1391, 22, 1341, 456);
    p.balas = src.copy(45, 603, 1326, 342);
    p.granadas = src.copy(1609, 603, 888, 342);
    p.furia = src.copy(387, 1034, 1546, 404);
    p.tapa = QColor(6, 27, 48);

    const QRect cuerpos[3] = {QRect(566, 208, 173, 145), QRect(773, 208, 170, 144), QRect(976, 208, 171, 145)};
    const int margen = 26;
    QVector<QRect> ventanas;
    for (const QRect &cuerpo : cuerpos) {
        const QRect ventana = cuerpo.adjusted(-margen, -margen, margen, margen).intersected(p.vidas.rect());
        if (ventana.isEmpty())
            continue;
        p.corazones.append(p.vidas.copy(ventana));
        p.posCorazon.append(ventana.topLeft());
        ventanas.append(ventana);
    }
    auto esPanel = [](const QColor &c) {
        return c.alpha() > 180 && c.blue() >= 45 && c.green() >= 24 && c.red() < 40;
    };
    auto esOro = [](const QColor &c) {
        return c.red() > 140 && c.green() > 90 && (c.green() - c.blue()) > 45 && c.red() > c.blue() + 25;
    };
    for (const QRect &v : ventanas) {
        for (int y = v.top(); y <= v.bottom(); ++y) {
            QColor izq;
            QColor der;
            bool hayIzq = false;
            bool hayDer = false;
            for (int x = v.left() - 1; x >= 0; --x) {
                const QColor c = p.vidas.pixelColor(x, y);
                if (esPanel(c)) {
                    izq = c;
                    hayIzq = true;
                    break;
                }
            }
            for (int x = v.right() + 1; x < p.vidas.width(); ++x) {
                const QColor c = p.vidas.pixelColor(x, y);
                if (esPanel(c)) {
                    der = c;
                    hayDer = true;
                    break;
                }
            }
            if (!hayIzq && !hayDer)
                continue;
            if (!hayIzq)
                izq = der;
            if (!hayDer)
                der = izq;
            const int span = qMax(1, v.width() - 1);
            for (int x = v.left(); x <= v.right(); ++x) {
                if (esOro(p.vidas.pixelColor(x, y)))
                    continue;
                const float t = float(x - v.left()) / float(span);
                p.vidas.setPixelColor(x, y, QColor(qRound(izq.red() * (1.f - t) + der.red() * t),
                                                   qRound(izq.green() * (1.f - t) + der.green() * t),
                                                   qRound(izq.blue() * (1.f - t) + der.blue() * t)));
            }
        }
    }

    p.rojo = QRect(65, 175, 851, 79);
    p.cifraVida = QRect(1012, 128, 280, 168);
    p.zonaBalas = QRect(425, 87, 450, 200);
    p.cifraBalas = QRect(915, 77, 360, 198);
    p.cifraGranadas = QRect(300, 88, 290, 168);
    p.rellenoFuria = QRect(393, 190, 1080, 112);
    p.cifraFuria = QRect(1240, 186, 280, 140);

    p.furiaDesde = QVector<int>(p.furia.height(), -1);
    p.furiaHasta = QVector<int>(p.furia.height(), -1);
    p.furiaOrigen = p.furia.width();
    p.furiaFin = 0;
    auto esRelleno = [](const QColor &c) {
        if (c.alpha() < 200 || c.blue() > 50)
            return false;
        if (c.red() > 200 && c.green() > 140 && c.blue() < 55)
            return true;
        return c.red() > 90 && c.green() < 110 && c.blue() < 40 && c.red() > c.green() + 40;
    };
    const int yTope = qMin(330, p.furia.height());
    const int xTope = qMin(1260, p.furia.width());
    for (int y = 160; y < yTope; ++y) {
        int primero = -1;
        int ultimo = -1;
        int cuenta = 0;
        for (int x = 420; x < xTope; ++x) {
            if (!esRelleno(p.furia.pixelColor(x, y)))
                continue;
            if (primero < 0)
                primero = x;
            ultimo = x;
            ++cuenta;
        }
        if (cuenta <= 30)
            continue;
        p.furiaDesde[y] = primero;
        p.furiaHasta[y] = ultimo;
        p.furiaOrigen = qMin(p.furiaOrigen, primero);
        p.furiaFin = qMax(p.furiaFin, ultimo);
    }
}

QImage componerMarcador(const PiezasHud &p, int vidas, float salud, int municion, int granadas, float furia) {
    if (p.vidas.isNull())
        return {};

    const float parte = qBound(0.f, salud, 1.f);
    const float furiaParte = qBound(0.f, furia, 1.f);
    const int balasVisibles = municion <= 0 ? 0 : qBound(1, qRound(6.f * municion / 20.f), 6);

    const int altoArriba = 64;
    const int sepX = 8;
    const int sepY = 6;
    auto pieza = [](const QImage &src, int alto) {
        return src.isNull() ? QImage() : src.scaledToHeight(alto, Qt::SmoothTransformation);
    };
    QImage vidasD = pieza(p.vidas, altoArriba);
    QImage saludD = pieza(p.salud, altoArriba);
    const int anchoFila = vidasD.width() + sepX + saludD.width();
    const int altoMedio = p.balas.height() > 0 && p.granadas.height() > 0
                              ? qMax(1, qRound(float(anchoFila - sepX) / (float(p.balas.width()) / float(p.balas.height())
                                                                          + float(p.granadas.width()) / float(p.granadas.height()))))
                              : 56;
    QImage balasD = pieza(p.balas, altoMedio);
    QImage granadasD = pieza(p.granadas, altoMedio);
    QImage furiaSrc = p.furia;
    if (furiaParte < 0.999f && p.furiaOrigen < p.furiaFin && !p.furiaDesde.isEmpty()) {
        const int corte = p.furiaOrigen + qRound(float(p.furiaFin - p.furiaOrigen) * furiaParte);
        for (int y = 0; y < furiaSrc.height() && y < p.furiaDesde.size(); ++y) {
            const int desde = p.furiaDesde.at(y);
            const int hasta = p.furiaHasta.at(y);
            if (desde < 0)
                continue;
            for (int x = qMax(corte, desde); x <= hasta && x < furiaSrc.width(); ++x)
                furiaSrc.setPixelColor(x, y, p.tapa);
        }
    }
    QImage furiaD = pieza(furiaSrc, altoMedio);

    const float ev = p.vidas.height() > 0 ? float(vidasD.height()) / float(p.vidas.height()) : 1.f;
    const float es = p.salud.height() > 0 ? float(saludD.height()) / float(p.salud.height()) : 1.f;
    const float eb = p.balas.height() > 0 ? float(balasD.height()) / float(p.balas.height()) : 1.f;
    const float eg = p.granadas.height() > 0 ? float(granadasD.height()) / float(p.granadas.height()) : 1.f;
    const float ef = p.furia.height() > 0 ? float(furiaD.height()) / float(p.furia.height()) : 1.f;

    const QRect cajaBalas = escalar(p.cifraBalas, eb);
    int tamComun = qBound(9, qRound(cajaBalas.height() * 0.52f), 18);
    {
        QFont letra(QStringLiteral("Georgia"), tamComun, QFont::Bold);
        letra.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
        QFontMetrics medida(letra);
        const QString muestra = QString::number(municion) + QStringLiteral("/20");
        while (tamComun > 8 && medida.horizontalAdvance(muestra) > cajaBalas.width() - 8) {
            --tamComun;
            letra.setPointSize(tamComun);
            medida = QFontMetrics(letra);
        }
    }

    QPainter pv(&vidasD);
    for (int i = 0; i < vidas && i < p.corazones.size(); ++i) {
        const QImage c = p.corazones.at(i).scaled(qRound(p.corazones.at(i).width() * ev),
                                                   qRound(p.corazones.at(i).height() * ev),
                                                   Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pv.drawImage(QPoint(qRound(p.posCorazon.at(i).x() * ev), qRound(p.posCorazon.at(i).y() * ev)), c);
    }
    pv.end();

    QPainter ps(&saludD);
    if (!p.rojo.isEmpty()) {
        const int lleno = qRound(p.rojo.width() * parte);
        const QRect hueco(p.rojo.x() + lleno, p.rojo.y(), p.rojo.width() - lleno, p.rojo.height());
        ps.fillRect(escalar(hueco, es), p.tapa);
    }
    pintarCifra(ps, escalar(p.cifraVida, es), QString::number(qRound(parte * 100.f)) + QStringLiteral("%"), tamComun);
    ps.end();

    QPainter pb(&balasD);
    if (balasVisibles < 6 && !p.zonaBalas.isEmpty()) {
        const int corte = p.zonaBalas.x() + p.zonaBalas.width() * balasVisibles / 6;
        pb.fillRect(escalar(QRect(corte, p.zonaBalas.y(), p.zonaBalas.right() - corte, p.zonaBalas.height()), eb), p.tapa);
    }
    pintarCifra(pb, escalar(p.cifraBalas, eb), QString::number(municion) + QStringLiteral("/20"), tamComun);
    pb.end();

    QPainter pg(&granadasD);
    pintarCifra(pg, escalar(p.cifraGranadas, eg), QString::number(qMax(0, granadas)) + QStringLiteral("/6"), tamComun);
    pg.end();

    QPainter pf(&furiaD);
    pintarCifra(pf, escalar(p.cifraFuria, ef), QString::number(qRound(furiaParte * 100.f)) + QStringLiteral("%"), tamComun);
    pf.end();

    const int ySalud = qMax(0, vidasD.height() - qRound(float(saludD.height()) * (357.f / 456.f)));
    const int filaMedio = qMax(balasD.height(), granadasD.height());
    const int ancho = qMax(anchoFila, qMax(balasD.width() + sepX + granadasD.width(), furiaD.width()));
    const int yBalas = vidasD.height() + sepY;
    const int yFuria = yBalas + filaMedio + sepY;
    const int altoLienzo = qMax(ySalud + saludD.height(), yFuria + furiaD.height());
    QImage lienzo(ancho, altoLienzo, QImage::Format_ARGB32);
    lienzo.fill(Qt::transparent);
    QPainter todo(&lienzo);
    todo.drawImage(0, 0, vidasD);
    todo.drawImage(vidasD.width() + sepX, ySalud, saludD);
    todo.drawImage(0, yBalas + (filaMedio - balasD.height()) / 2, balasD);
    todo.drawImage(balasD.width() + sepX, yBalas + (filaMedio - granadasD.height()) / 2, granadasD);
    todo.drawImage(0, yFuria, furiaD);
    todo.end();
    return lienzo;
}
