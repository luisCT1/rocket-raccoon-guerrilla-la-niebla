#include "MovOscilatorio.h"
#include <cmath>

MovOscilatorio::MovOscilatorio(float centro_, float amplitud_, float omega_, float fase_, bool vertical_)
    : centro(centro_), amplitud(amplitud_), omega(omega_), fase(fase_), vertical(vertical_), fijo(0.f) {}

void MovOscilatorio::setCoordenadaFija(float valor) {
    fijo = valor;
}

void MovOscilatorio::avanzar(float dt) {
    if (!activo) return;
    t += dt;
}

void MovOscilatorio::posicion(float &outX, float &outY) {
    const float s = centro + amplitud * std::sin(omega * t + fase);
    if (vertical) {
        outX = fijo;
        outY = s;
    } else {
        outX = s;
        outY = fijo;
    }
}
