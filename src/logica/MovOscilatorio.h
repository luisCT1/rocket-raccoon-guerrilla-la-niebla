#ifndef MOV_OSCILATORIO_H
#define MOV_OSCILATORIO_H

#include "ModeloFisico.h"

// Vaivén para el puente y las plataformas.
//   s(t) = centro + amplitud * sen(omega*t + fase)
// Oscila en un solo eje; la otra coordenada queda fija.

class MovOscilatorio : public ModeloFisico {
private:
    float centro;
    float amplitud;
    float omega;
    float fase;
    bool vertical;
    float fijo;
    float valor; //s(t) del cuadro actual

public:
    //Lanza std::invalid_argument si la amplitud es negativa
    MovOscilatorio(float centro, float amplitud, float omega, float fase, bool vertical, float fijo = 0.f);

    void setCoordenadaFija(float valor);
    void avanzar(float dt) override;
    void posicion(float &outX, float &outY) const override;
    void reiniciar() override;
};

#endif
