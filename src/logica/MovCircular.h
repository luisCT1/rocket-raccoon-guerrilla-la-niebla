#ifndef MOV_CIRCULAR_H
#define MOV_CIRCULAR_H

#include "ModeloFisico.h"

class MovCircular : public ModeloFisico {
private:
    float xc;
    float yc;
    float radio;
    float omega;
    float theta0;
    float theta;
    float px;
    float py;

    void recalcular();

public:
    MovCircular(float xc, float yc, float radio, float omega, float theta0 = 0.f);

    void avanzar(float dt) override;
    void posicion(float &outX, float &outY) const override;
    void reiniciar() override;

    float angulo() const;
    float velocidadAngular() const;
    void fijarOmega(float w);
    void fijarCentro(float x, float y);
};

#endif
