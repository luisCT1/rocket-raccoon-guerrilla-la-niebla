#ifndef SPRITES_H
#define SPRITES_H

#include <QHash>
#include <QPixmap>
#include <QString>

class Sprites {
private:
    QHash<QString, QPixmap> cache;
    QHash<QString, bool> faltan;
    QString carpeta;

public:
    explicit Sprites(const QString &carpetaBase);

    const QPixmap *obtener(const QString &nombre);
    int cantidadFaltantes() const;
};

#endif
