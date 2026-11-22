#ifndef ROCKET_H
#define ROCKET_H

#include "Entidad.h"

class Proyectil;

// Protagonista. Dispara, lanza granadas, hace dash, entra en furia y hace el ruido que oye el Capitán.
// Funciona en vista lateral (gravedad y salto) y en vista cenital (se mueve en los dos ejes).
// No conoce el teclado: la capa gráfica le dice qué hacer a través del Nivel.

class Rocket : public Entidad {
private:
    short vidas;
    float salud;
    float saludMax;
    bool cayo;
    short municion;
    short granadas;
    float barraFuria; //de 0 a 1; se llena al derrotar enemigos
    bool furioso;
    float tiempoFuria;
    float recargaDash;
    float bloqueoDisparo;
    float radioRuido;
    char direccion; //'L', 'R', 'U' o 'D'
    bool enSuelo;

    bool cenital;
    float intencionX; //hacia dónde quiere moverse en este cuadro (-1, 0 o 1)
    float intencionY;
    bool moviendose;
    float velY;
    float xAnterior; //posición al empezar el cuadro: el Nivel la usa para apoyarlo y bloquearlo
    float yAnterior;
    float tiempoDash;
    float dashX;
    float dashY;
    float invulnerable;
    float recargaDisparo;
    float recargaMunicion;
    float avisoGranada;
    bool golpePendiente;

public:
    Rocket(float x, float y);
    ~Rocket() override;

    void actualizar(float dt) override;
    short clase() const override;
    short estadoVisual() const override; //0 quieto, 1 camina, 2 en el aire, 3 dash
    void recibirDanio(float cantidad) override; //baja la vida; en cero se pierde una oportunidad

    void mover(char dir);  //'L', 'R', 'U' o 'D'. Se llama en cada cuadro mientras la tecla esté abajo
    bool saltar();
    //Devuelven nullptr si no puede (sin munición, recargando o bloqueado). Quien recibe el puntero es el dueño.
    Proyectil *disparar(float objX, float objY);
    Proyectil *lanzarGranada(float objX, float objY);
    bool dash(char dir);
    bool activarFuria();
    bool recibirDanio(); //false si estaba invulnerable y no perdió vida
    void perderVida();   //sin invulnerabilidad que valga (caída al barranco)
    void sumarFuria(float cantidad);
    float ruidoActual() const;
    bool lanzoGranadaReciente() const;
    bool consumirGolpe(); //true una sola vez por cada vida perdida

    //Lo que usa el Nivel para apoyarlo en el piso y las plataformas
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
