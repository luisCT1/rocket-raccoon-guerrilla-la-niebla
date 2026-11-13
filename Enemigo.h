#ifndef ENEMIGO_H
#define ENEMIGO_H

#include "Entidad.h"

class Rocket;
class Proyectil;

class Enemigo : public Entidad {
public:
    Enemigo(float x, float y);
    ~Enemigo() override;

    void actualizar(float dt) override;
    short clase() const override { return CLASE_ENEMIGO; }

    bool veA(const Rocket *r) const;
    Proyectil *disparar(float objX, float objY);
    virtual void recibirDanio(float danio);

    void  fijarObjetivo(const Rocket *r) { objetivo = r; }
    float getVida() const { return vida; }

protected:
    float vida;
    float velocidad;
    float alcanceVision;
    bool  esCorreo;
    char  direccion;
    const Rocket *objetivo;
};

#endif
