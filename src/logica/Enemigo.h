#ifndef ENEMIGO_H
#define ENEMIGO_H

#include "Entidad.h"

class Rocket;

//Soldado o correo. De aquí también va a heredar el Capitán (lo del compañero).

class Enemigo : public Entidad {
protected:
    short vida;
    float velocidad;
    float alcanceVision;
    bool esCorreo;
    char direccion;

public:
    Enemigo(float x, float y);
    virtual ~Enemigo();

    virtual void actualizar(float dt) override;
    bool veA(Rocket *r) const;
    void disparar(float dirX, float dirY);
    void recibirDanio(float cantidad);

    bool getEsCorreo() const;
    short getVida() const;
};

#endif
