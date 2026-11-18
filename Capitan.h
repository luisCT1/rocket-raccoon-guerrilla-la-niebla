#ifndef CAPITAN_H
#define CAPITAN_H

#include "Enemigo.h"
#include "AgenteTipos.h"
#include "Agente.h"

class MovCircular;

class Capitan : public Enemigo {
private:
    Agente *agente;
    bool duenoAgente;

    short fase;
    float vidaMax;
    float acumulador;
    float descanso;
    bool resultadoReportado;
    bool yaGolpeo;
    bool ordenCanon;
    float canonX;
    float canonY;
    bool cambioFase;
    short inicioAccion;
    MovCircular *giro;
    float factorVelocidad;
    bool derrotado;

    void iniciarAccion(short a, const Rocket *r);
    void finalizarAccion();
    void moverse(float dt);

public:
    Capitan(float x, float y, Agente *agente = nullptr);
    ~Capitan() override;

    void actualizar(float dt) override;
    short clase() const override;
    short estadoVisual() const override;
    bool esSolida() const override;
    void alChocar(Entidad &otra) override;
    void recibirDanio(float cantidad) override;

    void ciclo(const Rocket *r, float dt);
    void actualizarFase();

    bool alcanzaA(const Rocket *r) const;
    void confirmarGolpe();
    bool consumirOrdenCanon(float &objX, float &objY);
    bool consumirCambioFase();
    short consumirInicioAccion();
    void resultadoCanon(bool acerto);
    void fijarCanonDisponible(bool disponible);

    void fijarRetrasoPercepcion(float segundos);
    void fijarAgresividad(float a);
    void fijarTasaAprendizaje(float t);
    void fijarVelocidadAtaques(float factor);

    short getFase() const;
    bool estaDerrotado() const;
    float getVidaMax() const;
    short accionActual() const;
    float avanceAccion() const;
    bool giratorioActivo() const;
    void posicionGiratorio(float &gx, float &gy) const;
    bool trombaActiva() const;
    float radioTromba() const;
    const Agente *verAgente() const;
};

#endif
