#include "Marcador.h"

#include <QFont>
#include <QPainter>

namespace {
void pintarCifra(QPainter &pintor, const QRect &caja, const QString &texto) {
    if (caja.isEmpty())
        return;
    const int tam = qBound(9, qRound(caja.height() * 0.72f), 22);
    QFont letra(QStringLiteral("Georgia"), tam, QFont::Bold);
    letra.setStyleStrategy(QFont::PreferAntialias);
    letra.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
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

    const QRect corazonAbs[3] = {QRect(608, 230, 173, 143), QRect(816, 230, 169, 143), QRect(1018, 230, 169, 143)};
    for (const QRect &abs : corazonAbs) {
        const QRect local = abs.translated(-42, -22);
        if (!p.vidas.rect().contains(local))
            continue;
        p.corazones.append(p.vidas.copy(local));
        p.posCorazon.append(local.topLeft());
        for (int y = local.top(); y <= local.bottom(); ++y) {
            for (int x = local.left(); x <= local.right(); ++x)
                p.vidas.setPixelColor(x, y, p.tapa);
        }
    }

    p.rojo = QRect(65, 175, 851, 79);
    p.cifraVida = QRect(1009, 168, 250, 140);
    p.zonaBalas = QRect(425, 87, 450, 200);
    p.cifraBalas = QRect(915, 77, 360, 198);
    p.cifraGranadas = QRect(271, 97, 540, 160);
    p.rellenoFuria = QRect(393, 190, 1080, 112);
}

QImage componerMarcador(const PiezasHud &p, int vidas, float salud, int municion, int granadas, float furia) {
    if (p.vidas.isNull())
        return {};

    const float parte = qBound(0.f, salud, 1.f);
    const float furiaParte = qBound(0.f, furia, 1.f);
    const int balasVisibles = qBound(0, qRound(6.f * municion / 30.f), 6);

    auto pieza = [](const QImage &src, int alto) {
        return src.isNull() ? QImage() : src.scaledToHeight(alto, Qt::SmoothTransformation);
    };
    QImage vidasD = pieza(p.vidas, 86);
    QImage saludD = pieza(p.salud, 86);
    QImage balasD = pieza(p.balas, 64);
    QImage granadasD = pieza(p.granadas, 64);
    QImage furiaD = pieza(p.furia, 58);

    const float ev = p.vidas.height() > 0 ? 86.f / float(p.vidas.height()) : 1.f;
    const float es = p.salud.height() > 0 ? 86.f / float(p.salud.height()) : 1.f;
    const float eb = p.balas.height() > 0 ? 64.f / float(p.balas.height()) : 1.f;
    const float eg = p.granadas.height() > 0 ? 64.f / float(p.granadas.height()) : 1.f;
    const float ef = p.furia.height() > 0 ? 58.f / float(p.furia.height()) : 1.f;

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
    pintarCifra(ps, escalar(p.cifraVida, es), QString::number(qRound(parte * 100.f)) + QStringLiteral("%"));
    ps.end();

    QPainter pb(&balasD);
    if (balasVisibles < 6 && !p.zonaBalas.isEmpty()) {
        const int corte = p.zonaBalas.x() + p.zonaBalas.width() * balasVisibles / 6;
        pb.fillRect(escalar(QRect(corte, p.zonaBalas.y(), p.zonaBalas.right() - corte, p.zonaBalas.height()), eb), p.tapa);
    }
    pintarCifra(pb, escalar(p.cifraBalas, eb), QString::number(municion) + QStringLiteral("/30"));
    pb.end();

    QPainter pg(&granadasD);
    pintarCifra(pg, escalar(p.cifraGranadas, eg), QString::number(qMax(0, granadas)));
    pg.end();

    QPainter pf(&furiaD);
    if (!p.rellenoFuria.isEmpty()) {
        const int lleno = qRound(p.rellenoFuria.width() * furiaParte);
        const QRect hueco(p.rellenoFuria.x() + lleno, p.rellenoFuria.y(), p.rellenoFuria.width() - lleno, p.rellenoFuria.height());
        pf.fillRect(escalar(hueco, ef), p.tapa);
    }
    pf.end();

    const int filaVida = qMax(vidasD.height(), saludD.height());
    const int ancho = qMax(vidasD.width() + 8 + saludD.width(),
                           qMax(balasD.width() + 8 + granadasD.width(), furiaD.width()));
    const int yBalas = filaVida + 4;
    const int yFuria = yBalas + qMax(balasD.height(), granadasD.height()) + 4;
    QImage lienzo(ancho, yFuria + furiaD.height(), QImage::Format_ARGB32);
    lienzo.fill(Qt::transparent);
    QPainter todo(&lienzo);
    todo.drawImage(0, (filaVida - vidasD.height()) / 2, vidasD);
    todo.drawImage(vidasD.width() + 8, (filaVida - saludD.height()) / 2, saludD);
    todo.drawImage(0, yBalas, balasD);
    todo.drawImage(balasD.width() + 8, yBalas, granadasD);
    todo.drawImage(0, yFuria, furiaD);
    todo.end();
    return lienzo;
}
