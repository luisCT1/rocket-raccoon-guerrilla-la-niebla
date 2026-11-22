#include "MovCircular.h"
#include <cmath>
#include <stdexcept>

MovCircular::MovCircular(float xc_, float yc_, float radio_, float omega_, float theta0_)
    : xc(xc_), yc(yc_), radio(radio_), omega(omega_), theta0(theta0_), theta(theta0_), px(0.f), py(0.f) {
    if (radio_ <= 0.f)
        throw std::invalid_argument("MovCircular: el radio debe ser mayor que 0");

    theta = std::fmod(theta, DOS_PI_F);
    if (theta < 0.f) theta += DOS_PI_F;
    recalcular();
}

void MovCircular::avanzar(float dt) {
    if (!activo) return;

    t += dt;
    theta += omega * dt;

    if (theta >= DOS_PI_F || theta < 0.f) {
        theta = std::fmod(theta, DOS_PI_F);
        if (theta < 0.f) theta += DOS_PI_F;
    }
    recalcular();
}

void MovCircular::posicion(float &outX, float &outY) const {
    outX = px;
    outY = py;
}

void MovCircular::reiniciar() {
    ModeloFisico::reiniciar();
    theta = std::fmod(theta0, DOS_PI_F);
    if (theta < 0.f) theta += DOS_PI_F;
    recalcular();
}

float MovCircular::angulo() const { return theta; }
float MovCircular::velocidadAngular() const { return omega; }

void MovCircular::fijarOmega(float w) {
    omega = w;
}

void MovCircular::fijarCentro(float x, float y) {
    xc = x;
    yc = y;
    recalcular();
}

void MovCircular::recalcular() {
    px = xc + radio * std::cos(theta);
    py = yc + radio * std::sin(theta);
}
