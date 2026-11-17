#include "Sprites.h"

#include <algorithm>

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

int Sprites::contarFrames(const QString &nombre) {
    auto it = numFrames.constFind(nombre);
    if (it != numFrames.constEnd()) return it.value();
    int n = 0;
    while (n < 64 && obtener(QString("%1_%2").arg(nombre).arg(n + 1))) ++n;
    numFrames.insert(nombre, n);
    return n;
}

const QPixmap *Sprites::obtenerAnimado(const QString &nombre, float t, float fps, bool unaVez) {
    const int n = contarFrames(nombre);
    if (n == 0) return obtener(nombre);
    int i = static_cast<int>(t * fps);
    i = unaVez ? std::min(i, n - 1) : i % n;
    return obtener(QString("%1_%2").arg(nombre).arg(i + 1));
}
