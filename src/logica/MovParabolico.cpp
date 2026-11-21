#include "MovParabolico.h"
#include <cmath>
#include <stdexcept>

MovParabolico::MovParabolico(float x0_, float y0_, float v0_, float alfa_, float g_)
    : x0(x0_), y0(y0_), v0(v0_), alfa(alfa_), g(g_),
      velX(v0_ * std::cos(alfa_)),
      velY(-v0_ * std::sin(alfa_)) {
    if (v0_ < 0.f || g_ < 0.f)
        throw std::invalid_argument("MovParabolico: v0 y g no pueden ser negativos");
}

MovParabolico *MovParabolico::haciaObjetivo(float x0, float y0, float xObj, float yObj, float tiempoVuelo, float g) {
    if (tiempoVuelo <= 0.f)
        throw std::invalid_argument("MovParabolico: el tiempo de vuelo debe ser mayor que 0");

    const float vx = (xObj - x0) / tiempoVuelo;
    const float vy = (yObj - y0 - 0.5f * g * tiempoVuelo * tiempoVuelo) / tiempoVuelo;
    return new MovParabolico(x0, y0, std::sqrt(vx * vx + vy * vy), std::atan2(-vy, vx), g);
}

void MovParabolico::avanzar(float dt) {
    if (!activo) return;
    t += dt;
}

void MovParabolico::posicion(float &outX, float &outY) const {
    outX = x0 + velX * t;
    outY = y0 + velY * t + 0.5f * g * t * t;
}

bool MovParabolico::aterrizo(float ySuelo) const {

    const float y = y0 + velY * t + 0.5f * g * t * t;
    return y >= ySuelo;
}

float MovParabolico::getV0() const { return v0; }
float MovParabolico::getAlfa() const { return alfa; }
