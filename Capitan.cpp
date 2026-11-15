#include "Capitan.h"
#include "Aprendizaje.h"
#include "MovCircular.h"
#include "Rocket.h"
#include <stdexcept>

namespace {
constexpr float ANCHO_CAPITAN = 60.f;
constexpr float ALTO_CAPITAN = 90.f;
constexpr float VIDA_CAPITAN = 1000.f;
constexpr float INTERVALO_DECISION = 0.2f;
constexpr float PAUSA_ENTRE_ACCIONES = 0.8f;
constexpr float VELOCIDAD = 140.f;
constexpr float DISTANCIA_MINIMA = 30.f;
constexpr float RADIO_GIRO = 110.f;
constexpr float RADIO_GOLPE_GIRO = 35.f;
const float OMEGA_FASE[3] = { 4.f, 5.f, 6.5f };
constexpr float TROMBA_MAX = 260.f;
constexpr float TROMBA_GROSOR = 45.f;
constexpr float TROMBA_ALTURA = 80.f;
constexpr float TIEMPO_HABITO = 2.f;
constexpr float DISTANCIA_LEJOS = 300.f;
}

Capitan::Capitan(float x_, float y_, Aprendizaje *memoria_)
    : Enemigo(x_, y_),
      aprendizaje(memoria_ ? memoria_ : new Aprendizaje()),
      duenoAprendizaje(memoria_ == nullptr),
      percepcion(),
      razonamiento(aprendizaje),
      accion(),
      fase(FASE_ACOSA), vidaMax(VIDA_CAPITAN), acumulador(0.f), descanso(0.f), tiempoAlto(0.f), tiempoLejos(0.f),
      estimuloPrevio(ESTIM_NINGUNO), resultadoReportado(true), yaGolpeo(false),
      ordenCanon(false), canonX(0.f), canonY(0.f), cambioFase(false), inicioAccion(ACC_NINGUNA),
      giro(nullptr), factorVelocidad(1.f), derrotado(false) {
    ancho = ANCHO_CAPITAN;
    alto = ALTO_CAPITAN;
    vida = VIDA_CAPITAN;
    seguirModelos = false;

    giro = new MovCircular(getCentroX(), getCentroY(), RADIO_GIRO, OMEGA_FASE[0]);
    giro->activar(false);
    agregarModelo(giro);
}

Capitan::~Capitan() {
    if (duenoAprendizaje) delete aprendizaje;
}

void Capitan::actualizar(float dt) {
    if (derrotado) return;

    Entidad::actualizar(dt);
    razonamiento.actualizarRecargas(dt);
    accion.actualizar(dt);

    if (accion.estaTerminada() && !resultadoReportado) {
        finalizarAccion();
        descanso = PAUSA_ENTRE_ACCIONES / factorVelocidad;
    } else if (descanso > 0.f) {
        descanso -= dt;
    }

    acumulador += dt;
    const bool libre = accion.estaTerminada() && descanso <= 0.f;
    if (acumulador >= INTERVALO_DECISION - 1.0e-4f || libre) {
        ciclo(objetivo, acumulador);
        acumulador = 0.f;
    }
    moverse(dt);
}

void Capitan::ciclo(const Rocket *r, float dt) {
    if (derrotado || !r) return;

    percepcion.percibir(r, getCentroX(), getCentroY(), dt);

    const short e = percepcion.estimulo();
    if (e != estimuloPrevio) {
        if (e == ESTIM_DASH) aprendizaje->registrarHabito(HAB_DASH);
        else if (e == ESTIM_GRANADA) aprendizaje->registrarHabito(HAB_GRANADA);
        estimuloPrevio = e;
    }
    tiempoAlto = percepcion.rocketAlto() ? tiempoAlto + dt : 0.f;
    if (tiempoAlto >= TIEMPO_HABITO) { aprendizaje->registrarHabito(HAB_ALTO); tiempoAlto = 0.f; }
    tiempoLejos = percepcion.getDistancia() > DISTANCIA_LEJOS ? tiempoLejos + dt : 0.f;
    if (tiempoLejos >= TIEMPO_HABITO) { aprendizaje->registrarHabito(HAB_LEJOS); tiempoLejos = 0.f; }

    if (!accion.estaTerminada()) return;

    if (!resultadoReportado) {
        finalizarAccion();
        descanso = PAUSA_ENTRE_ACCIONES / factorVelocidad;
    }
    if (descanso > 0.f) return;

    const short a = razonamiento.decidir(&percepcion, fase);
    if (a != ACC_NINGUNA) iniciarAccion(a, r);
}

void Capitan::iniciarAccion(short a, const Rocket *r) {
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
        giro->fijarCentro(getCentroX(), getCentroY());
        giro->reiniciar();
        giro->activar(true);
    }
}

void Capitan::finalizarAccion() {
    giro->activar(false);
    const short a = accion.getActual();
    if (a == ACC_NINGUNA || resultadoReportado) return;
    resultadoReportado = true;
    if (a != ACC_CANON && a != ACC_REPLEGARSE)
        aprendizaje->resultado(a, accion.acerto());
}

void Capitan::moverse(float dt) {
    if (!objetivo || accion.estaTerminada()) return;

    const float dx = objetivo->getCentroX() - getCentroX();
    const float sentido = dx >= 0.f ? 1.f : -1.f;
    direccion = dx >= 0.f ? 'R' : 'L';

    const short a = accion.getActual();
    if (a == ACC_ACOSAR) {
        if (dx * sentido > DISTANCIA_MINIMA) x += sentido * VELOCIDAD * factorVelocidad * dt;
    } else if (a == ACC_REPLEGARSE) {
        x -= sentido * VELOCIDAD * factorVelocidad * dt;
    }
    if (x < minX) x = minX;
    else if (x > maxX) x = maxX;

    if (giro->estaActivo()) giro->fijarCentro(getCentroX(), getCentroY());
}

short Capitan::clase() const {
    return CLASE_CAPITAN;
}

short Capitan::estadoVisual() const {
    if (derrotado) return 99;
    return accion.estaTerminada() ? 0 : static_cast<short>(accion.getActual() + 1);
}

bool Capitan::esSolida() const {
    return !derrotado;
}

void Capitan::alChocar(Entidad &otra) {
    (void)otra;
}

void Capitan::recibirDanio(float cantidad) {
    if (derrotado || cantidad <= 0.f) return;
    vida -= cantidad;
    if (vida <= 0.f) {
        vida = 0.f;
        derrotado = true;
        giro->activar(false);
    }
    actualizarFase();
}

void Capitan::actualizarFase() {
    const float fraccion = vida / vidaMax;
    const short nueva = fraccion > 0.66f ? FASE_ACOSA : (fraccion > 0.33f ? FASE_CANON : FASE_AGRESIVO);
    if (nueva != fase) {
        fase = nueva;
        cambioFase = true;
        giro->fijarOmega(OMEGA_FASE[fase - 1] * factorVelocidad);
    }
}

bool Capitan::alcanzaA(const Rocket *r) const {
    if (derrotado || !r || yaGolpeo || accion.estaTerminada()) return false;

    switch (accion.getActual()) {
    case ACC_ACOSAR:
        return colisiona(r);
    case ACC_GIRATORIO: {
        float gx, gy;
        giro->posicion(gx, gy);
        const float dx = r->getCentroX() - gx;
        const float dy = r->getCentroY() - gy;
        const float alcance = RADIO_GOLPE_GIRO + r->getAncho() * 0.5f;
        return dx * dx + dy * dy <= alcance * alcance;
    }
    case ACC_TROMBA: {
        const float frente = radioTromba();
        float adx = r->getCentroX() - getCentroX();
        if (adx < 0.f) adx = -adx;
        float ady = r->getCentroY() - getCentroY();
        if (ady < 0.f) ady = -ady;
        return adx <= frente && adx >= frente - TROMBA_GROSOR && ady < TROMBA_ALTURA;
    }
    default:
        return false;
    }
}

void Capitan::confirmarGolpe() {
    yaGolpeo = true;
    accion.marcarAcierto();
}

bool Capitan::consumirOrdenCanon(float &objX, float &objY) {
    if (!ordenCanon) return false;
    ordenCanon = false;
    objX = canonX;
    objY = canonY;
    return true;
}

bool Capitan::consumirCambioFase() {
    const bool c = cambioFase;
    cambioFase = false;
    return c;
}

short Capitan::consumirInicioAccion() {
    const short a = inicioAccion;
    inicioAccion = ACC_NINGUNA;
    return a;
}

void Capitan::resultadoCanon(bool acerto) {
    aprendizaje->resultado(ACC_CANON, acerto);
}

void Capitan::fijarCanonDisponible(bool disponible) {
    razonamiento.bloquear(ACC_CANON, !disponible);
}

void Capitan::fijarRetrasoPercepcion(float segundos) { percepcion.fijarRetraso(segundos); }
void Capitan::fijarAgresividad(float a) { razonamiento.fijarAgresividad(a); }
void Capitan::fijarTasaAprendizaje(float t) { aprendizaje->fijarTasa(t); }

void Capitan::fijarVelocidadAtaques(float factor) {
    if (factor <= 0.3f || factor > 3.f)
        throw std::invalid_argument("Capitan: el factor de velocidad debe estar en (0.3, 3]");
    factorVelocidad = factor;
    razonamiento.fijarFactorRecarga(1.f / factor);
    giro->fijarOmega(OMEGA_FASE[fase - 1] * factor);
}

short Capitan::getFase() const { return fase; }
bool Capitan::estaDerrotado() const { return derrotado; }
float Capitan::getVidaMax() const { return vidaMax; }

short Capitan::accionActual() const {
    return accion.estaTerminada() ? static_cast<short>(ACC_NINGUNA) : accion.getActual();
}

float Capitan::avanceAccion() const { return accion.getAvance(); }

bool Capitan::giratorioActivo() const {
    return !derrotado && giro->estaActivo();
}

void Capitan::posicionGiratorio(float &gx, float &gy) const {
    giro->posicion(gx, gy);
}

bool Capitan::trombaActiva() const {
    return !derrotado && !accion.estaTerminada() && accion.getActual() == ACC_TROMBA;
}

float Capitan::radioTromba() const {
    return trombaActiva() ? TROMBA_MAX * accion.getAvance() : 0.f;
}

const Aprendizaje *Capitan::memoria() const { return aprendizaje; }
const Percepcion &Capitan::verPercepcion() const { return percepcion; }
