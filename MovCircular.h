#ifndef MOVCIRCULAR_H
#define MOVCIRCULAR_H

#include "ModeloFisico.h"

class MovCircular : public ModeloFisico {
public:

    MovCircular(float xc, float yc, float radio, float omega, float theta0 = 0.0f);

    void avanzar(float dt) override;
    void posicion(float &x, float &y) const override;
    void reiniciar() override;

    float angulo() const { return theta; }
    float velocidadAngular() const { return omega; }
    void  fijarOmega(float w) { omega = w; }
    void  fijarCentro(float x, float y);

private:
    void recalcular();

    float xc, yc, radio, omega, theta0, theta;
    float px, py;
};

#endif // MOVCIRCULAR_H
