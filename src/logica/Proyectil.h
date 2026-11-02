#ifndef PROYECTIL_H
#define PROYECTIL_H

#include "Entidad.h"

//Bala o granada. Si es de Rocket, el nivel la usa para pegarle a los enemigos.

class Proyectil : public Entidad {
protected:
    float danio;
    float radioExplosion;
    bool deRocket;
    bool explotado;

public:
    Proyectil(float x, float y, float radioExplosion, bool deRocket);
    ~Proyectil() override;

    void actualizar(float dt) override;
    bool aterrizo() const;
    void explotar();

    float getDanio() const;
    bool esDeRocket() const;
    bool yaExplotado() const;
};

#endif
