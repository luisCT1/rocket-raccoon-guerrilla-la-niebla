#ifndef CAPITAN_H
#define CAPITAN_H

#include "Enemigo.h"
#include "AgenteTipos.h"
#include "Percepcion.h"
#include "Razonamiento.h"
#include "Accion.h"

class Aprendizaje;
class MovCircular;

class Capitan : public Enemigo {
public:
    Capitan(float x, float y, Aprendizaje *memoria = nullptr);
    ~Capitan() override;

    void  actualizar(float dt) override;
    short clase() const override { return CLASE_CAPITAN; }
    short estadoVisual() const override;
    void  alChocar(Entidad &otra) override;
    void  recibirDanio(float danio) override;

    void ciclo(const Rocket *r, float dt);
    void actualizarFase();

    bool  golpeaA(const Rocket *r);
    bool  consumirOrdenCanon(float &objX, float &objY);
    bool  consumirCambioFase();
    short consumirInicioAccion();
    void  resultadoCanon(bool acerto);
    void  fijarCanonDisponible(bool disponible);
    void  fijarLimites(float minimoX, float maximoX);

    void fijarRetrasoPercepcion(float segundos);
    void fijarAgresividad(float a);
    void fijarTasaAprendizaje(float t);
    void fijarVelocidadAtaques(float factor);

    short getFase() const { return fase; }
    bool  estaDerrotado() const { return derrotado; }
    float getVidaMax() const { return vidaMax; }
    short accionActual() const;
    float avanceAccion() const { return accion.getAvance(); }
    bool  giratorioActivo() const;
    void  posicionGiratorio(float &gx, float &gy) const;
    bool  trombaActiva() const;
    float radioTromba() const;
    const Aprendizaje *memoria() const { return aprendizaje; }
    const Percepcion &verPercepcion() const { return percepcion; }

private:
    void iniciarAccion(short a, const Rocket *r);
    void finalizarAccion();
    void moverse(float dt);

    Aprendizaje *aprendizaje;
    bool         duenoAprendizaje;
    Percepcion   percepcion;
    Razonamiento razonamiento;
    Accion       accion;

    short fase;
    float vidaMax;
    float acumulador;
    float descanso;
    float tiempoAlto, tiempoLejos;
    short estimuloPrevio;
    bool  resultadoReportado;
    bool  yaGolpeo;
    bool  ordenCanon;
    float canonX, canonY;
    bool  cambioFase;
    short inicioAccion;
    MovCircular *giro;
    float minX, maxX;
    float factorVelocidad;
    bool  derrotado;
};

#endif
