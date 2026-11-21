#ifndef MOV_OSCILATORIO_H
#define MOV_OSCILATORIO_H

#include "ModeloFisico.h"

class MovOscilatorio : public ModeloFisico {
private:
    float centro;
    float amplitud;
    float omega;
    float fase;
    bool vertical;
    float fijo;
    float valor;

public:
    MovOscilatorio(float centro, float amplitud, float omega, float fase, bool vertical, float fijo = 0.f);

    void setCoordenadaFija(float valor);
    void avanzar(float dt) override;
    void posicion(float &outX, float &outY) const override;
    void reiniciar() override;
};

#endif
