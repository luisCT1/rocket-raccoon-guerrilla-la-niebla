#include "NivelPenon.h"
#include "Capitan.h"
#include "Estructura.h"
#include "Proyectil.h"
#include "Rocket.h"
#include "AgenteTipos.h"
#include "Eventos.h"

static const float ANCHO_MUNDO = 1280.0f;
static const float ALTO_MUNDO  = 720.0f;
static const float ROCKET_X  = 200.0f,  ROCKET_Y  = 600.0f;
static const float CAPITAN_X = 950.0f,  CAPITAN_Y = 595.0f;
static const float CANON_X   = 1160.0f, CANON_Y   = 615.0f;
static const float LIMITE_IZQ = 120.0f, LIMITE_DER = 1100.0f;
static const float PLATAFORMA_X[3] = { 380.0f, 700.0f, 1000.0f };
static const float PLATAFORMA_Y[3] = { 480.0f, 380.0f, 480.0f };
static const float BANDERA_X = 700.0f, BANDERA_Y = 330.0f;
static const float BANDERA_RADIO = 60.0f;

NivelPenon::NivelPenon(short nivelDificultad, Aprendizaje *memoria)
    : capitan(nullptr), dificultad(nivelDificultad), banderaPuesta(false), canon(nullptr), canonAvisado(false)
{
    anchoMundo = ANCHO_MUNDO;
    altoMundo  = ALTO_MUNDO;
    disparosCanon.reserve(8);

    Rocket *r = new Rocket(ROCKET_X, ROCKET_Y);
    agregarEntidad(r);
    fijarJugador(r);

    for (short i = 0; i < 3; ++i) {
        Estructura *p = new Estructura(EST_PLATAFORMA, PLATAFORMA_X[i], PLATAFORMA_Y[i]);
        p->fijarOscila(false);
        agregarEntidad(p);
    }

    canon = new Estructura(EST_CANON, CANON_X, CANON_Y);
    agregarEntidad(canon);

    capitan = new Capitan(CAPITAN_X, CAPITAN_Y, memoria);
    agregarEntidad(capitan);
    capitan->fijarObjetivo(r);
    capitan->fijarLimites(LIMITE_IZQ, LIMITE_DER);
    dificultad.aplicar(capitan);
}

void NivelPenon::actualizar(float dt)
{
    if (terminado) return;

    Nivel::actualizar(dt);

    if (!canonAvisado && canon->estaDestruida()) {
        canonAvisado = true;
        capitan->fijarCanonDisponible(false);
        emitirEvento(EV_EXPLOSION);
    }

    short accion = capitan->consumirInicioAccion();
    if (accion == ACC_GIRATORIO || accion == ACC_TROMBA) emitirEvento(EV_ATAQUE_GIRATORIO);

    float objX, objY;
    if (capitan->consumirOrdenCanon(objX, objY)) {
        Proyectil *p = canon->disparar(objX, objY);
        if (p != nullptr) {
            agregarEntidad(p);
            disparosCanon.push_back(p);
            emitirEvento(EV_CANON_DISPARA);
        }
    }
    seguirDisparos();

    if (capitan->golpeaA(rocket)) {
        rocket->recibirDanio();
        emitirEvento(EV_GOLPE_A_ROCKET);
    }
    if (capitan->consumirCambioFase()) cambiarFase();

    if (rocket->getVidas() <= 0) {
        terminar(false);
    } else if (capitan->estaDerrotado()) {
        const Entidad *jugador = rocket;
        float dx = jugador->getX() - BANDERA_X;
        float dy = jugador->getY() - BANDERA_Y;
        if (dx * dx + dy * dy <= BANDERA_RADIO * BANDERA_RADIO) ponerBandera();
    }
}

void NivelPenon::seguirDisparos()
{
    const Entidad *jugador = rocket;
    for (std::size_t i = 0; i < disparosCanon.size(); ) {
        Proyectil *p = disparosCanon[i];
        if (p->haExplotado() || !p->estaViva()) {
            float dx = jugador->getX() - p->getX();
            float dy = jugador->getY() - p->getY();
            float alcance = p->getRadioExplosion() + jugador->getAncho() * 0.5f;
            capitan->resultadoCanon(dx * dx + dy * dy <= alcance * alcance);
            emitirEvento(EV_EXPLOSION);
            disparosCanon[i] = disparosCanon.back();
            disparosCanon.pop_back();
        } else {
            ++i;
        }
    }
}

void NivelPenon::cambiarFase()
{
    emitirEvento(EV_CAMBIO_FASE_JEFE);
}

void NivelPenon::ponerBandera()
{
    if (terminado) return;
    banderaPuesta = true;
    terminar(true);
}

float NivelPenon::getBanderaX() const { return BANDERA_X; }
float NivelPenon::getBanderaY() const { return BANDERA_Y; }
