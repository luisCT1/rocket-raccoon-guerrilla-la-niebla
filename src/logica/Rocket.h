#ifndef ROCKET_H
#define ROCKET_H

#include "Entidad.h"

class Proyectil;

class Rocket : public Entidad {
private:
    short vidas;
    float salud;
    float saludMax;
    bool cayo;
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
    bool agachado;

    bool cenital;
    float intencionX;
    float intencionY;
    bool moviendose;
    float velY;
    float xAnterior;
    float yAnterior;
    float tiempoDash;
    float dashX;
    float dashY;
    float invulnerable;
    float recargaDisparo;
    float avisoGranada;
    bool golpePendiente;
    bool recogidaPendiente;
    short tipoRecogida;
    bool pasoPendiente;
    float pasoAcum;
    bool agachadoAntes;
    bool agacharPendiente;
    bool aterrizajePendiente;

public:
    Rocket(float x, float y);
    ~Rocket() override;

    void actualizar(float dt) override;
    short clase() const override;
    short estadoVisual() const override;
    void recibirDanio(float cantidad) override;

    void mover(char dir);
    bool saltar();

    Proyectil *disparar(float objX, float objY);
    Proyectil *lanzarGranada(float objX, float objY);
    bool dash(char dir);
    bool activarFuria();
    void fijarAgachado(bool agacha);
    bool estaAgachado() const;
    bool recibirDanio();
    void perderVida();
    void sumarFuria(float cantidad);
    bool recogerVida();
    bool recogerMunicion(short cantidad);
    bool recogerGranadas(short cantidad);
    bool recogerFuria(float cantidad);
    float ruidoActual() const;
    bool lanzoGranadaReciente() const;
    bool consumirGolpe();
    short consumirRecogida();
    bool consumirPaso();
    bool consumirAgacharse();
    bool consumirAterrizaje();

    void fijarCenital(bool c);
    void aterrizarEn(float ySuperficie);
    void soltarSuelo();
    void reaparecerEn(float nx, float ny);

    short getVidas() const;
    float getSalud() const;
    float getSaludMax() const;
    bool cayoAlVacio() const;
    short getMunicion() const;
    short getGranadas() const;
    float getBarraFuria() const;
    float getVelY() const;
    float getXAnterior() const;
    float getYAnterior() const;
    char getDireccion() const;
    bool estaFurioso() const;
    bool estaEnSuelo() const;
    bool esInvulnerable() const;
    bool dashListo() const;
};

#endif
