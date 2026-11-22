#ifndef CAPITAN_H
#define CAPITAN_H

#include "Enemigo.h"
#include "AgenteTipos.h"
#include <array>

class MovCircular;

class Capitan : public Enemigo {
public:
    struct Memoria {
        std::array<short, HAB_NUM> conteoHabitos;
        std::array<float, ACC_NUM> pesosAccion;
        std::array<float, ACC_NUM> bonoAccion;
        Memoria();
    };

    static constexpr float NO_DISPONIBLE = -1000.0f;

private:
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
    bool sablePendiente;
    bool sableSonado;
    bool vozPendiente;
    bool impactoPendiente;
    float vozEspera;
    short inicioAccion;
    MovCircular *giro;
    float factorVelocidad;
    bool derrotado;
    bool encontrado;

    short estimuloVisible;
    float intensidadEstimulo;
    float distanciaRocket;
    short ladoRocket;
    bool rocketEnAlto;
    float radioOido;
    float retrasoReaccion;
    short estimuloPendiente;
    float intensidadPendiente;
    float esperaReaccion;

    std::array<float, ACC_NUM> recargas;
    std::array<float, ACC_NUM> sinUsar;
    std::array<bool, ACC_NUM> bloqueadas;
    float agresividad;
    float factorRecarga;

    short idAccion;
    float accionRestante, accionDuracion;
    float destinoX, destinoY;
    bool accionFinalizada, accionAcierto;

    Memoria memoriaPropia;
    Memoria *memoria;
    float tasaAprendizaje;
    short estimuloPrevio;
    float tiempoAlto, tiempoLejos;
    float bajoFuego;
    bool seAcerca;

    void finalizarAccion();
    void moverse(float dt);
    void pedirVoz();
    void recalcularBonos();

public:
    Capitan(float x, float y, Memoria *memoria = nullptr);
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
    bool consumirSable();
    bool consumirVoz();
    bool consumirImpacto();
    short consumirInicioAccion();
    void resultadoCanon(bool acerto);
    void fijarCanonDisponible(bool disponible);

    short getFase() const;
    bool estaDerrotado() const;
    bool estaPersiguiendo() const;
    float getVidaMax() const;
    bool giratorioActivo() const;
    void posicionGiratorio(float &gx, float &gy) const;
    bool trombaActiva() const;
    bool muestraSable() const;
    float radioTromba() const;

    void fijarDificultad(short nivel);

    void percibir(const Rocket *r, float dt);
    short estimulo() const;
    float getIntensidad() const;
    float getDistancia() const;
    short lado() const;
    bool rocketAlto() const;
    bool dashReciente() const;
    bool granadaReciente() const;
    void fijarRetrasoPercepcion(float segundos);
    void fijarRadioOido(float radio);
    float getRetrasoPercepcion() const;

    short decidir() const;
    float puntaje(short accion) const;
    void iniciarRecarga(short accion);
    void actualizarRecargas(float dt);
    bool estaLista(short accion) const;
    void bloquear(short accion, bool bloqueada);
    void fijarAgresividad(float a);
    void fijarVelocidadAtaques(float factor);
    float getAgresividad() const;
    float getVelocidadAtaques() const;

    void ejecutarAccion(short accion, const Rocket *r);
    void actualizarAccion(float dt);
    bool accionTerminada() const;
    short accionActual() const;
    float avanceAccion() const;

    void registrarHabito(short habito);
    void observarHabitos(float dt);
    void resultado(short accion, bool acerto);
    float peso(short accion) const;
    void fijarTasaAprendizaje(float t);
    float getTasa() const;
    short conteo(short habito) const;
    short habitoDominante() const;
    void olvidarTodo();
    const Memoria *verMemoria() const;
};

#endif
