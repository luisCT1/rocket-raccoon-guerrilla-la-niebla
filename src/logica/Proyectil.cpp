#include "Proyectil.h"
#include "MovParabolico.h"

Proyectil::Proyectil(float x_, float y_, float radioExplosion_, bool deRocket_)
    : Entidad(x_, y_, 16.f, 16.f),
      danio(deRocket_ ? 25.f : 15.f),
      radioExplosion(radioExplosion_),
      deRocket(deRocket_),
      explotado(false) {}

Proyectil::~Proyectil() {}

void Proyectil::actualizar(float dt) {
    if (explotado || !viva) return;
    Entidad::actualizar(dt);
    if (aterrizo()) {
        explotar();
    }
}

bool Proyectil::aterrizo() const {
    //Suelo provisional; cuando el nivel tenga su propio piso lo cambiamos
    constexpr float Y_SUELO = 520.f;
    for (ModeloFisico *m : modelos) {
        auto *parab = dynamic_cast<MovParabolico *>(m);
        if (parab && parab->aterrizo(Y_SUELO)) return true;
    }
    return y >= Y_SUELO;
}

void Proyectil::explotar() {
    if (explotado) return;
    explotado = true;
    matar();
}

float Proyectil::getDanio() const { return danio; }
bool Proyectil::esDeRocket() const { return deRocket; }
bool Proyectil::yaExplotado() const { return explotado; }
