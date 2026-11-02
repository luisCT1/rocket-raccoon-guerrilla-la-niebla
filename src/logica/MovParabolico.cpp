#include "MovParabolico.h"
#include <cmath>

MovParabolico::MovParabolico(float x0_, float y0_, float v0_, float alfa_, float g_)
    : x0(x0_), y0(y0_), v0(v0_), alfa(alfa_), g(g_) {}

void MovParabolico::avanzar(float dt) {
    if (!activo) return;
    t += dt;
}

void MovParabolico::posicion(float &outX, float &outY) {
    outX = x0 + v0 * std::cos(alfa) * t;
    outY = y0 + v0 * std::sin(alfa) * t - 0.5f * g * t * t;
}

bool MovParabolico::aterrizo(float ySuelo) const {
    const float y = y0 + v0 * std::sin(alfa) * t - 0.5f * g * t * t;
    return y <= ySuelo;
}
