#ifndef ENEMIGO_H
#define ENEMIGO_H

#include "Entidad.h"

class Rocket;
class Proyectil;

class Enemigo : public Entidad {
protected:
    float vida;
    float velocidad;
    float alcanceVision;
    bool esCorreo;
    bool marcha;
    char direccion;
    const Rocket *objetivo;
    bool cenital;
    float minX;
    float maxX;
    float origenX;
    float recargaDisparo;
    bool persiguiendo;
    bool avanzo;
    short aspecto;
    float tiempoArma;

    void notarDesplazamiento(float x0, float y0);

public:
    Enemigo(float x, float y);
    virtual ~Enemigo();

    void actualizar(float dt) override;
    short clase() const override;
    short estadoVisual() const override;
    void alChocar(Entidad &otra) override;
    void recibirDanio(float cantidad) override;

    bool veA(const Rocket *r) const;
    bool listoParaDisparar() const;

    Proyectil *disparar(float objX, float objY);

    void fijarObjetivo(const Rocket *r);
    void fijarCenital(bool c);
    void fijarLimites(float minimoX, float maximoX);
    void fijarAlcanceVision(float alcance);
    void fijarCorreo(bool c);
    void fijarMarcha(bool m);
    void fijarAspecto(short a);

    bool getEsCorreo() const;
    short getAspecto() const;
    bool seMovio() const;
    bool esMontado() const;
    bool suenaElPaso() const;
    bool llevaArma() const;
    float getVida() const;
    char getDireccion() const;
};

#endif
