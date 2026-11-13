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
public:
    Estructura(short tipo, float x, float y);

    void actualizar(float dt) override;
    short clase() const override { return CLASE_ESTRUCTURA; }
    short estadoVisual() const override { return destruida ? 1 : 0; }
    void alChocar(Entidad &otra) override;
    void recibirDanio(float danio);

    Proyectil *disparar(float objX, float objY);
    bool listaParaDisparar() const;

    bool detecta(const Rocket *r) const;

    bool  estaDestruida() const { return destruida; }
    short getTipo() const { return tipo; }
    float getResistencia() const { return resistencia; }
    float anguloVigilancia() const;
    float getRadioCono() const { return radioCono; }
    float getApertura() const { return apertura; }

    void fijarCadencia(float segundos);
    void fijarCono(float radio, float apertura);
    void fijarVelocidadGiro(float omega);
    void fijarOscila(bool o) { oscila = o; }

protected:
    short tipo;
    float resistencia;
    bool  destruida;
    bool  oscila;
    float cadencia;
    float tiempoDisparo;
    float radioCono;
    float apertura;
    float cos2Media;
    MovCircular *giro;
};

#endif
