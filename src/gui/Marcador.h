#ifndef MARCADOR_H
#define MARCADOR_H

#include <QImage>
#include <QPoint>
#include <QRect>
#include <QVector>

struct PiezasHud {
    QImage vidas;
    QVector<QImage> corazones;
    QVector<QPoint> posCorazon;
    QImage salud;
    QRect rojo;
    QRect cifraVida;
    QImage balas;
    QRect zonaBalas;
    QRect cifraBalas;
    QImage granadas;
    QRect cifraGranadas;
    QImage furia;
    QRect rellenoFuria;
    QRect cifraFuria;
    QVector<int> furiaDesde;
    QVector<int> furiaHasta;
    int furiaOrigen;
    int furiaFin;
    QColor tapa;
};

void prepararMarcador(const QImage &hoja, PiezasHud &piezas);
QImage componerMarcador(const PiezasHud &piezas, int vidas, float salud, int municion, int granadas, float furia);

#endif
