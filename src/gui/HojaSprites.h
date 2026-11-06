#ifndef HOJA_SPRITES_H
#define HOJA_SPRITES_H

#include <QPixmap>
#include <QRect>
#include <QVector>

//Recorta una hoja en los cuadros que le pasemos y los deja del alto pedido.

QVector<QPixmap> cortarHoja(const QPixmap &hoja, const QVector<QRect> &cuadros, int alto);

#endif
