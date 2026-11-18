#ifndef PROYECTIL_H
#define PROYECTIL_H

#include "Entidad.h"

// Bala, granada o bala de cañón.
// Directo (bala): explota al tocar a un rival o al llegar al suelo.
// Bombeado (granada, cañón): vuela por encima de todo y explota al llegar a su destino.
// El daño no lo reparte el proyectil: al explotar, el Nivel daña a lo que esté dentro del radio.

class Proyectil : public Entidad {
protected:
    float danio;
    float radioExplosion;
    bool deRocket;
    bool explotado;
    bool bombeado;
    bool explosionPendiente; //el Nivel todavía no ha repartido el daño de esta explosión
    float tiempoVida;
    float edad;
    float ySuelo;
    float destinoX; //dónde va a caer (centro). Sirve para dibujar el aviso en el piso
    float destinoY;

public:
    //Lanza std::invalid_argument si radioExplosion es negativo
    Proyectil(float x, float y, float radioExplosion, bool deRocket);
    ~Proyectil() override;

    void actualizar(float dt) override;
    short clase() const override;
    void alChocar(Entidad &otra) override;

    bool aterrizo() const;
    void explotar();
    bool consumirExplosion(); //true una sola vez, justo después de explotar

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
