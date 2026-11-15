#include "Sprites.h"

Sprites::Sprites(const QString &carpetaBase) : carpeta(carpetaBase) {}

const QPixmap *Sprites::obtener(const QString &nombre) {
    auto it = cache.constFind(nombre);
    if (it != cache.constEnd()) return &it.value();
    if (faltan.contains(nombre)) return nullptr;

    QPixmap img(carpeta + nombre + ".png");
    if (img.isNull()) {
        faltan.insert(nombre, true);
        return nullptr;
    }
    return &cache.insert(nombre, img).value();
}

int Sprites::cantidadFaltantes() const { return faltan.size(); }
