#ifndef AGENTE_H
#define AGENTE_H

#include "AgenteTipos.h"
#include <array>

class Rocket;

class Agente {
public:
    Agente();

    void percibir(const Rocket *r, float capX, float capY, float dt);

    short estimulo() const { return tipoEstimulo; }
    float getIntensidad() const { return intensidad; }
    float getDistancia() const { return distancia; }
    short lado() const { return ladoRocket; }
    bool  rocketAlto() const { return alto; }
    bool  dashReciente() const { return tipoEstimulo == ESTIM_DASH; }
    bool  granadaReciente() const { return tipoEstimulo == ESTIM_GRANADA; }

    void fijarRetraso(float segundos);
    void fijarRadioOido(float radio);

    short decidir(short fase) const;
    float puntaje(short accion, short fase) const;

    void iniciarRecarga(short accion);
    void actualizarRecargas(float dt);
    bool estaLista(short accion) const;
    void bloquear(short accion, bool bloqueada);

    void fijarAgresividad(float a);
    void fijarFactorRecarga(float f);

    static constexpr float NO_DISPONIBLE = -1000.0f;

    void ejecutar(short accion, const Rocket *r);
    void actualizarAccion(float dt);

    bool  accionTerminada() const { return terminada; }
    bool  acerto() const { return acierto; }
    void  marcarAcierto() { acierto = true; }
    short getAccion() const { return actual; }
    float getDestinoX() const { return destinoX; }
    float getDestinoY() const { return destinoY; }
    float getAvanceAccion() const;
    bool  consumirInicio();

    void registrarHabito(short habito);
    void observarHabitos(float dt);
    void resultado(short accion, bool acerto);
    float peso(short accion) const;

    void  fijarTasa(float t);
    float getTasa() const { return tasa; }
    short conteo(short habito) const;
    short habitoDominante() const;
    void  olvidarTodo();

    void reiniciarIntento();

private:
    void recalcularBonos();

    short tipoEstimulo;
    float intensidad;
    float distancia;
    short ladoRocket;
    bool  alto;
    float radioOido;
    float retraso;
    short pendiente;
    float intensidadPendiente;
    float espera;

    std::array<float, ACC_NUM> recargas;
    std::array<float, ACC_NUM> sinUsar;
    std::array<bool, ACC_NUM> bloqueada;
    float agresividad;
    float factorRecarga;

    short actual;
    float restante, duracion;
    float destinoX, destinoY;
    bool  terminada, acierto, inicioPendiente;

    std::array<short, HAB_NUM> conteoHabitos;
    std::array<float, ACC_NUM> pesosAccion;
    std::array<float, ACC_NUM> bonoAccion;
    float tasa;
    short estimuloPrevio;
    float tiempoAlto, tiempoLejos;
};

#endif
