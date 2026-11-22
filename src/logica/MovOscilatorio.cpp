#include "MovOscilatorio.h"
#include <cmath>
#include <stdexcept>

MovOscilatorio::MovOscilatorio(float centro_, float amplitud_, float omega_, float fase_, bool vertical_, float fijo_)
    : centro(centro_), amplitud(amplitud_), omega(omega_), fase(fase_), vertical(vertical_), fijo(fijo_),
      valor(centro_ + amplitud_ * std::sin(fase_)) {
    if (amplitud_ < 0.f)
        throw std::invalid_argument("MovOscilatorio: la amplitud no puede ser negativa");
}

void MovOscilatorio::setCoordenadaFija(float v) {
    fijo = v;
}

void MovOscilatorio::avanzar(float dt) {
    if (!activo) return;
    t += dt;
    valor = centro + amplitud * std::sin(omega * t + fase);
}

void MovOscilatorio::posicion(float &outX, float &outY) const {
    if (vertical) {
        outX = fijo;
        outY = valor;
    } else {
        outX = valor;
        outY = fijo;
    }
}

void MovOscilatorio::reiniciar() {
    ModeloFisico::reiniciar();
    valor = centro + amplitud * std::sin(fase);
}
