#ifndef ROCKET_H
#define ROCKET_H

#include "Entidad.h"

class Rocket : public Entidad {
public:
    Rocket(float x, float y);

    void actualizar(float dt) override;
    short clase() const override { return CLASE_ROCKET; }
    void mover(char direccion);
    void saltar();
    void disparar(float objX, float objY);
    void lanzarGranada(float objX, float objY);
    void dash(char direccion);
    void activarFuria();
    void recibirDanio();

    float ruidoActual() const;
    bool  lanzoGranadaReciente() const;
    char  getDireccion() const { return direccion; }
    short getVidas() const { return vidas; }
    bool  estaFurioso() const { return furioso; }

private:
    short vidas, municion, granadas;
    float barraFuria, tiempoFuria, recargaDash, bloqueoDisparo, radioRuido;
    bool  furioso, enSuelo;
    char  direccion;
};

#endif
