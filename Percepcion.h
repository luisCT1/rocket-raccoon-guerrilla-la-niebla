#ifndef PERCEPCION_H
#define PERCEPCION_H

#include "AgenteTipos.h"

class Rocket;

class Percepcion {
public:
    Percepcion();

    void percibir(const Rocket *r, float capX, float capY, float dt);

    short estimulo() const { return tipoEstimulo; }
    float getIntensidad() const { return intensidad; }
    float getDistancia() const { return distancia; }
    short lado() const { return ladoRocket; }
    bool  rocketAlto() const { return alto; }
    bool  dashReciente() const { return tipoEstimulo == ESTIM_DASH; }
    bool  granadaReciente() const { return tipoEstimulo == ESTIM_GRANADA; }

    void fijarRetraso(float segundos);
    void fijarRadioOido(float radio);

private:
    short tipoEstimulo;
    float intensidad;
    float distancia;
    short ladoRocket;
    bool  alto;
    float radioOido;
    float retraso;
    short pendiente;
    float intensidadPendiente;
    float espera;
};

#endif
