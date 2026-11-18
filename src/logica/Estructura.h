#ifndef ESTRUCTURA_H
#define ESTRUCTURA_H

#include "Entidad.h"

class Proyectil;
class Rocket;
class MovCircular;

enum TipoEstructura : short {
    EST_CARRETA = 0,
    EST_BARRIL,
    EST_POLVORIN,
    EST_BANDERA,
    EST_CANON,
    EST_TORRE,
    EST_PLATAFORMA,
    EST_PUENTE,
    EST_NUM_TIPOS
};

class Estructura : public Entidad {
protected:
    short tipo;
    float resistencia;
    bool destruida;
    bool destruccionPendiente;
    float cadencia;
    float tiempoDisparo;
    float radioCono;
    float apertura;
    float cos2Media;
    MovCircular *giro;

public:
    Estructura(short tipo, float x, float y);

    void actualizar(float dt) override;
    short clase() const override;
    short estadoVisual() const override;
    bool esSolida() const override;
    void recibirDanio(float cantidad) override;

    Proyectil *disparar(float objX, float objY);
    bool listaParaDisparar() const;

    bool detecta(const Rocket *r) const;

    bool estaDestruida() const;
    bool consumirDestruccion();
    short getTipo() const;
    float getResistencia() const;
    float anguloVigilancia() const;
    float getRadioCono() const;
    float getApertura() const;

    void fijarCadencia(float segundos);
    void fijarCono(float radio, float apertura);
    void fijarVelocidadGiro(float omega);
};

#endif
