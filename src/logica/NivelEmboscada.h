#ifndef NIVEL_EMBOSCADA_H
#define NIVEL_EMBOSCADA_H

#include "Nivel.h"

//Nivel 1. Emboscada al convoy. Lo vamos a enchufar cuando el combate base esté firme.

class NivelEmboscada : public Nivel {
private:
    float posCamara;
    short partesRestantes;
    bool correoEscapo;
    float intervaloOleada;
    float tiempoOleada;

public:
    NivelEmboscada();
    ~NivelEmboscada() override;

    void actualizar(float dt) override;
    bool victoria() override;
    bool derrota() override;

    void generarOleada();
    void moverCamara();
};

#endif
