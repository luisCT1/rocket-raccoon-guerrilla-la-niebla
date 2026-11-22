#ifndef HOJA_SPRITES_H
#define HOJA_SPRITES_H

#include <QPixmap>
#include <QRect>
#include <QVector>

QVector<QPixmap> cortarHoja(const QPixmap &hoja, const QVector<QRect> &cuadros, int alto);

#endif
