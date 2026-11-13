#include "Capitan.h"
#include "Aprendizaje.h"
#include "MovCircular.h"
#include "Proyectil.h"
#include "Rocket.h"
#include <stdexcept>

static const float ANCHO_CAPITAN      = 60.0f;
static const float ALTO_CAPITAN       = 90.0f;
static const float VIDA_CAPITAN       = 300.0f;
static const float INTERVALO_DECISION = 0.2f;
static const float PAUSA_ENTRE_ACCIONES = 0.8f;
static const float VELOCIDAD          = 140.0f;
static const float DISTANCIA_MINIMA   = 30.0f;
static const float RADIO_GIRO         = 110.0f;
static const float RADIO_GOLPE_GIRO   = 35.0f;
static const float OMEGA_FASE[3]      = { 4.0f, 5.0f, 6.5f };
static const float TROMBA_MAX         = 260.0f;
static const float TROMBA_GROSOR      = 45.0f;
static const float TROMBA_ALTURA      = 80.0f;
static const float TIEMPO_HABITO      = 2.0f;
static const float DISTANCIA_LEJOS    = 300.0f;

Capitan::Capitan(float x, float y, Aprendizaje *memoria)
    : Enemigo(x, y),
      aprendizaje(memoria ? memoria : new Aprendizaje()),
      duenoAprendizaje(memoria == nullptr),
      percepcion(),
      razonamiento(aprendizaje),
      accion(),
      fase(FASE_ACOSA), vidaMax(VIDA_CAPITAN), acumulador(0.0f), descanso(0.0f), tiempoAlto(0.0f), tiempoLejos(0.0f),
      estimuloPrevio(ESTIM_NINGUNO), resultadoReportado(true), yaGolpeo(false),
      ordenCanon(false), canonX(0.0f), canonY(0.0f), cambioFase(false), inicioAccion(ACC_NINGUNA),
      giro(nullptr), minX(-1.0e9f), maxX(1.0e9f), factorVelocidad(1.0f), derrotado(false)
{
    ancho = ANCHO_CAPITAN;
    alto  = ALTO_CAPITAN;
    vida  = VIDA_CAPITAN;

    giro = new MovCircular(x, y, RADIO_GIRO, OMEGA_FASE[0]);
    giro->activar(false);
    agregarModelo(giro);
}

Capitan::~Capitan()
{
    if (duenoAprendizaje) delete aprendizaje;
}

void Capitan::actualizar(float dt)
{
    if (derrotado) return;

    Entidad::actualizar(dt);
    razonamiento.actualizarRecargas(dt);
    accion.actualizar(dt);

    if (accion.estaTerminada() && !resultadoReportado) {
        finalizarAccion();
        descanso = PAUSA_ENTRE_ACCIONES / factorVelocidad;
    } else if (descanso > 0.0f) {
        descanso -= dt;
    }

    acumulador += dt;
    bool libre = accion.estaTerminada() && descanso <= 0.0f;
    if (acumulador >= INTERVALO_DECISION - 1.0e-4f || libre) {
        ciclo(objetivo, acumulador);
        acumulador = 0.0f;
    }
    moverse(dt);
}

void Capitan::ciclo(const Rocket *r, float dt)
{
    if (derrotado || r == nullptr) return;

    percepcion.percibir(r, x, y, dt);

    short e = percepcion.estimulo();
    if (e != estimuloPrevio) {
        if (e == ESTIM_DASH) aprendizaje->registrarHabito(HAB_DASH);
        else if (e == ESTIM_GRANADA) aprendizaje->registrarHabito(HAB_GRANADA);
        estimuloPrevio = e;
    }
    tiempoAlto = percepcion.rocketAlto() ? tiempoAlto + dt : 0.0f;
    if (tiempoAlto >= TIEMPO_HABITO) { aprendizaje->registrarHabito(HAB_ALTO); tiempoAlto = 0.0f; }
    tiempoLejos = percepcion.getDistancia() > DISTANCIA_LEJOS ? tiempoLejos + dt : 0.0f;
    if (tiempoLejos >= TIEMPO_HABITO) { aprendizaje->registrarHabito(HAB_LEJOS); tiempoLejos = 0.0f; }

    if (!accion.estaTerminada()) return;

    if (!resultadoReportado) {
        finalizarAccion();
        descanso = PAUSA_ENTRE_ACCIONES / factorVelocidad;
    }
    if (descanso > 0.0f) return;

    short a = razonamiento.decidir(&percepcion, fase);
    if (a != ACC_NINGUNA) iniciarAccion(a, r);
}

void Capitan::iniciarAccion(short a, const Rocket *r)
{
    accion.ejecutar(a, r);
    razonamiento.iniciarRecarga(a);
    resultadoReportado = false;
    yaGolpeo = false;
    inicioAccion = a;

    if (a == ACC_CANON) {
        ordenCanon = true;
        canonX = accion.getDestinoX();
        canonY = accion.getDestinoY();
    } else if (a == ACC_GIRATORIO) {
        giro->fijarCentro(x, y);
        giro->reiniciar();
        giro->activar(true);
    }
}

void Capitan::finalizarAccion()
{
    giro->activar(false);
    short a = accion.getActual();
    if (a == ACC_NINGUNA || resultadoReportado) return;
    resultadoReportado = true;
    if (a != ACC_CANON && a != ACC_REPLEGARSE)
        aprendizaje->resultado(a, accion.acerto());
}

void Capitan::moverse(float dt)
{
    if (objetivo == nullptr || accion.estaTerminada()) return;

    const Entidad *o = objetivo;
    float dx = o->getX() - x;
    float sentido = dx >= 0.0f ? 1.0f : -1.0f;
    direccion = dx >= 0.0f ? 'd' : 'a';

    short a = accion.getActual();
    if (a == ACC_ACOSAR) {
        if (dx * sentido > DISTANCIA_MINIMA) x += sentido * VELOCIDAD * factorVelocidad * dt;
    } else if (a == ACC_REPLEGARSE) {
        x -= sentido * VELOCIDAD * factorVelocidad * dt;
    }
    if (x < minX) x = minX; else if (x > maxX) x = maxX;

    if (giro->estaActivo()) giro->fijarCentro(x, y);
}

void Capitan::recibirDanio(float danio)
{
    if (derrotado || danio <= 0.0f) return;
    vida -= danio;
    if (vida <= 0.0f) {
        vida = 0.0f;
        derrotado = true;
        giro->activar(false);
    }
    actualizarFase();
}

void Capitan::actualizarFase()
{
    float fraccion = vida / vidaMax;
    short nueva = fraccion > 0.66f ? FASE_ACOSA : (fraccion > 0.33f ? FASE_CANON : FASE_AGRESIVO);
    if (nueva != fase) {
        fase = nueva;
        cambioFase = true;
        giro->fijarOmega(OMEGA_FASE[fase - 1] * factorVelocidad);
    }
}

void Capitan::alChocar(Entidad &otra)
{
    if (otra.clase() != CLASE_PROYECTIL) return;
    Proyectil &p = static_cast<Proyectil &>(otra);
    if (p.esDeRocket() && !p.haExplotado()) {
        recibirDanio(p.getDanio());
        p.explotar();
    }
}

bool Capitan::golpeaA(const Rocket *r)
{
    if (derrotado || r == nullptr || yaGolpeo || accion.estaTerminada()) return false;

    const Entidad *o = r;
    bool toca = false;
    switch (accion.getActual()) {
    case ACC_ACOSAR:
        toca = colisiona(o);
        break;
    case ACC_GIRATORIO: {
        float gx, gy;
        giro->posicion(gx, gy);
        float dx = o->getX() - gx, dy = o->getY() - gy;
        float alcance = RADIO_GOLPE_GIRO + o->getAncho() * 0.5f;
        toca = dx * dx + dy * dy <= alcance * alcance;
        break;
    }
    case ACC_TROMBA: {
        float frente = radioTromba();
        float adx = o->getX() - x; if (adx < 0.0f) adx = -adx;
        float ady = o->getY() - y; if (ady < 0.0f) ady = -ady;
        toca = adx <= frente && adx >= frente - TROMBA_GROSOR && ady < TROMBA_ALTURA;
        break;
    }
    default:
        break;
    }
    if (toca) {
        yaGolpeo = true;
        accion.marcarAcierto();
    }
    return toca;
}

bool Capitan::consumirOrdenCanon(float &objX, float &objY)
{
    if (!ordenCanon) return false;
    ordenCanon = false;
    objX = canonX;
    objY = canonY;
    return true;
}

bool Capitan::consumirCambioFase()
{
    bool c = cambioFase;
    cambioFase = false;
    return c;
}

short Capitan::consumirInicioAccion()
{
    short a = inicioAccion;
    inicioAccion = ACC_NINGUNA;
    return a;
}

void Capitan::resultadoCanon(bool acerto)
{
    aprendizaje->resultado(ACC_CANON, acerto);
}

void Capitan::fijarCanonDisponible(bool disponible)
{
    razonamiento.bloquear(ACC_CANON, !disponible);
}

void Capitan::fijarLimites(float minimoX, float maximoX)
{
    if (minimoX > maximoX) throw std::invalid_argument("Capitan: limites invertidos");
    minX = minimoX;
    maxX = maximoX;
}

void Capitan::fijarRetrasoPercepcion(float segundos) { percepcion.fijarRetraso(segundos); }
void Capitan::fijarAgresividad(float a)              { razonamiento.fijarAgresividad(a); }
void Capitan::fijarTasaAprendizaje(float t)          { aprendizaje->fijarTasa(t); }

void Capitan::fijarVelocidadAtaques(float factor)
{
    if (factor <= 0.3f || factor > 3.0f)
        throw std::invalid_argument("Capitan: el factor de velocidad debe estar en (0.3, 3]");
    factorVelocidad = factor;
    razonamiento.fijarFactorRecarga(1.0f / factor);
    giro->fijarOmega(OMEGA_FASE[fase - 1] * factor);
}

short Capitan::estadoVisual() const
{
    if (derrotado) return 99;
    return accion.estaTerminada() ? 0 : static_cast<short>(accion.getActual() + 1);
}

short Capitan::accionActual() const
{
    return accion.estaTerminada() ? static_cast<short>(ACC_NINGUNA) : accion.getActual();
}

bool Capitan::giratorioActivo() const
{
    return !derrotado && giro->estaActivo();
}

void Capitan::posicionGiratorio(float &gx, float &gy) const
{
    giro->posicion(gx, gy);
}

bool Capitan::trombaActiva() const
{
    return !derrotado && !accion.estaTerminada() && accion.getActual() == ACC_TROMBA;
}

float Capitan::radioTromba() const
{
    return trombaActiva() ? TROMBA_MAX * accion.getAvance() : 0.0f;
}
