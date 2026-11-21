#ifndef PROYECTIL_H
#define PROYECTIL_H

#include "Entidad.h"

class Proyectil : public Entidad {
protected:
    float danio;
    float radioExplosion;
    bool deRocket;
    bool explotado;
    bool bombeado;
    bool explosionPendiente;
    float tiempoVida;
    float edad;
    float ySuelo;
    float destinoX;
    float destinoY;

public:

    Proyectil(float x, float y, float radioExplosion, bool deRocket);
    ~Proyectil() override;

    void actualizar(float dt) override;
    short clase() const override;
    void alChocar(Entidad &otra) override;

    bool aterrizo() const;
    void explotar();
    bool consumirExplosion();

    void fijarDanio(float d);
    void fijarBombeado(bool b);
    void fijarTiempoVida(float segundos);
    void fijarSuelo(float y);
    void fijarDestino(float cx, float cy);

    float getDanio() const;
    float getRadioExplosion() const;
    float getDestinoX() const;
    float getDestinoY() const;
    bool esDeRocket() const;
    bool esBombeado() const;
    bool yaExplotado() const;
};

#endif
