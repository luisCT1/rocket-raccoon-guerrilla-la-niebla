#ifndef MOV_OSCILATORIO_H
#define MOV_OSCILATORIO_H

#include "ModeloFisico.h"

//Vaivén para puentes o plataformas. Todavía no está en el .pro actual, pero queda listo.

class MovOscilatorio : public ModeloFisico {
private:
    float centro;
    float amplitud;
    float omega;
    float fase;
    bool vertical;
    float fijo;

public:
    MovOscilatorio(float centro, float amplitud, float omega, float fase, bool vertical);

    void setCoordenadaFija(float valor);
    void avanzar(float dt) override;
    void posicion(float &outX, float &outY) override;
};

#endif
