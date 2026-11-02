#ifndef ROCKET_H
#define ROCKET_H

#include "Entidad.h"

class Proyectil;

//Protagonista. Dispara, dash, furia y el ruido que después usa el Capitán.

class Rocket : public Entidad {
private:
    short vidas;
    short municion;
    short granadas;
    float barraFuria;
    bool furioso;
    float tiempoFuria;
    float recargaDash;
    float bloqueoDisparo;
    float radioRuido;
    char direccion;
    bool enSuelo;

public:
    Rocket(float x, float y);
    ~Rocket() override;

    void actualizar(float dt) override;

    void mover(char dir);
    void saltar();
    Proyectil *disparar(float dirX, float dirY);
    Proyectil *lanzarGranada(float dirX, float dirY);
    void dash(char dir);
    void activarFuria();
    void recibirDanio();
    float ruidoActual() const;

    short getVidas() const;
    short getMunicion() const;
    bool estaFurioso() const;
};

#endif
