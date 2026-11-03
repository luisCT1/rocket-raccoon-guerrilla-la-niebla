#include "MovCircular.h"
#include <cmath>
#include <stdexcept>

MovCircular::MovCircular(float xc, float yc, float radio, float omega, float theta0)
    : xc(xc), yc(yc), radio(radio), omega(omega), theta0(theta0), theta(theta0), px(0.0f), py(0.0f)
{
    if (radio <= 0.0f)
        throw std::invalid_argument("MovCircular: el radio debe ser mayor que 0");

    theta = std::fmod(theta, DOS_PI_F);
    if (theta < 0.0f) theta += DOS_PI_F;
    recalcular();
}

void MovCircular::avanzar(float dt)
{
    if (!activo) return;

    t     += dt;
    theta += omega * dt;

    if (theta >= DOS_PI_F || theta < 0.0f) {
        theta = std::fmod(theta, DOS_PI_F);
        if (theta < 0.0f) theta += DOS_PI_F;
    }
    recalcular();
}

void MovCircular::posicion(float &x, float &y) const
{
    x = px;
    y = py;
}

void MovCircular::reiniciar()
{
    ModeloFisico::reiniciar();
    theta = std::fmod(theta0, DOS_PI_F);
    if (theta < 0.0f) theta += DOS_PI_F;
    recalcular();
}

void MovCircular::fijarCentro(float x, float y)
{
    xc = x;
    yc = y;
    recalcular();
}

void MovCircular::recalcular()
{
    px = xc + radio * std::cos(theta);
    py = yc + radio * std::sin(theta);
}
