#include "HojaSprites.h"

QVector<QPixmap> cortarHoja(const QPixmap &hoja, const QVector<QRect> &cuadros, int alto) {
    QVector<QPixmap> salida;
    salida.reserve(cuadros.size());
    for (const QRect &r : cuadros) {
        QPixmap recorte = hoja.copy(r);
        if (alto > 0 && recorte.height() > 0) {
            recorte = recorte.scaledToHeight(alto, Qt::SmoothTransformation);
        }
        salida.push_back(recorte);
    }
    return salida;
}
