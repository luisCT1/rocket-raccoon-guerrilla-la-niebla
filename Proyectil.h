#ifndef PROYECTIL_H
#define PROYECTIL_H

#include "Entidad.h"

class Proyectil : public Entidad {
public:
    Proyectil(float x, float y, float v0, float alfa);
    Proyectil(float x, float y, float xObjetivo, float yObjetivo, float tiempoVuelo, float g);

    void actualizar(float dt) override;
    short clase() const override { return CLASE_PROYECTIL; }
    bool aterrizo() const;
    void explotar();

    void  fijarDanio(float d)    { danio = d; }
    void  fijarDeRocket(bool b)  { deRocket = b; }
    float getDanio() const       { return danio; }
    float getRadioExplosion() const { return radioExplosion; }
    bool  esDeRocket() const     { return deRocket; }
    bool  haExplotado() const    { return explotado; }

protected:
    float danio;
    float radioExplosion;
    bool  deRocket;
    bool  explotado;
};

#endif
