#include "NivelPenon.h"
#include "Capitan.h"
#include "Estructura.h"
#include "Proyectil.h"
#include "Rocket.h"
#include "MovOscilatorio.h"
#include "AgenteTipos.h"
#include "Eventos.h"

namespace {
constexpr float ANCHO_MUNDO = 1280.f;
constexpr float ALTO_MUNDO = 720.f;
constexpr float SUELO = 640.f;
constexpr float ROCKET_X = 176.f;
constexpr float CAPITAN_X = 900.f;
constexpr float CANON_X = 1125.f;
constexpr float LIMITE_IZQ = 120.f;
constexpr float LIMITE_DER = 1040.f;
constexpr float CIMA_X = 620.f;
constexpr float CIMA_Y = 440.f;
const float LATERAL_X[2] = { 400.f, 840.f };
constexpr float LATERAL_Y = 540.f;
constexpr float LATERAL_AMPLITUD = 30.f;
constexpr float LATERAL_OMEGA = 1.5f;
constexpr float BANDERA_X = 700.f;
constexpr float BANDERA_Y = 408.f;
constexpr float BANDERA_RADIO = 60.f;
}

NivelPenon::NivelPenon(short nivelDificultad, Aprendizaje *memoria)
    : capitan(nullptr), dificultad(nivelDificultad), banderaPuesta(false), canon(nullptr), canonAvisado(false) {
    anchoMundo = ANCHO_MUNDO;
    altoMundo = ALTO_MUNDO;
    cenital = false;
    ySuelo = SUELO;
    disparosCanon.reserve(8);

    Rocket *r = new Rocket(ROCKET_X, 0.f);
    r->setPosicion(ROCKET_X, SUELO - r->getAlto());
    agregarEntidad(r);

    Estructura *cima = new Estructura(EST_PLATAFORMA, CIMA_X, CIMA_Y);
    agregarEntidad(cima);
    registrarPlataforma(cima);

    for (short i = 0; i < 2; ++i) {
        Estructura *p = new Estructura(EST_PLATAFORMA, LATERAL_X[i], LATERAL_Y);
        p->agregarModelo(new MovOscilatorio(LATERAL_Y, LATERAL_AMPLITUD, LATERAL_OMEGA, i == 0 ? 0.f : PI_F, true, LATERAL_X[i]));
        agregarEntidad(p);
        registrarPlataforma(p);
    }

    canon = new Estructura(EST_CANON, CANON_X, 0.f);
    canon->setPosicion(CANON_X, SUELO - canon->getAlto());
    agregarEntidad(canon);

    capitan = new Capitan(CAPITAN_X, 0.f, memoria);
    capitan->setPosicion(CAPITAN_X, SUELO - capitan->getAlto());
    agregarEntidad(capitan);
    capitan->fijarLimites(LIMITE_IZQ, LIMITE_DER);
    dificultad.aplicar(capitan);
}

NivelPenon::~NivelPenon() {}

void NivelPenon::actualizar(float dt) {
    if (terminado) return;
    Nivel::actualizar(dt);

    if (!canonAvisado && canon->estaDestruida()) {
        canonAvisado = true;
        capitan->fijarCanonDisponible(false);
        emitirEvento(EV_EXPLOSION);
    }

    const short accion = capitan->consumirInicioAccion();
    if (accion == ACC_GIRATORIO) emitirEvento(EV_ATAQUE_GIRATORIO);
    else if (accion == ACC_TROMBA) emitirEvento(EV_TROMBA);

    float objX, objY;
    if (capitan->consumirOrdenCanon(objX, objY)) {
        Proyectil *p = canon->disparar(objX, objY);
        if (p) {
            agregarEntidad(p);
            disparosCanon.push_back(p);
            emitirEvento(EV_CANON_DISPARA);
        }
    }
    seguirDisparos();

    if (capitan->alcanzaA(rocket) && rocket->recibirDanio()) capitan->confirmarGolpe();
    notificarGolpe();

    if (capitan->consumirCambioFase()) cambiarFase();

    if (rocket->getVidas() <= 0) {
        terminar(false);
    } else if (capitan->estaDerrotado()) {
        const float dx = rocket->getCentroX() - BANDERA_X;
        const float dy = rocket->getCentroY() - BANDERA_Y;
        if (dx * dx + dy * dy <= BANDERA_RADIO * BANDERA_RADIO) ponerBandera();
    }
}

void NivelPenon::seguirDisparos() {
    for (std::size_t i = 0; i < disparosCanon.size(); ) {
        Proyectil *p = disparosCanon[i];
        if (p->yaExplotado() || !p->estaViva()) {
            const float dx = rocket->getCentroX() - p->getCentroX();
            const float dy = rocket->getCentroY() - p->getCentroY();
            const float alcance = p->getRadioExplosion() + rocket->getAncho() * 0.5f;
            capitan->resultadoCanon(dx * dx + dy * dy <= alcance * alcance);
            disparosCanon[i] = disparosCanon.back();
            disparosCanon.pop_back();
        } else {
            ++i;
        }
    }
}

void NivelPenon::alBorrar(Entidad *e) {
    for (std::size_t i = 0; i < disparosCanon.size(); ++i) {
        if (disparosCanon[i] == e) {
            disparosCanon[i] = disparosCanon.back();
            disparosCanon.pop_back();
            break;
        }
    }
    Nivel::alBorrar(e);
}

void NivelPenon::cambiarFase() {
    emitirEvento(EV_CAMBIO_FASE_JEFE);
}

void NivelPenon::ponerBandera() {
    if (terminado) return;
    banderaPuesta = true;
    terminar(true);
}

bool NivelPenon::victoria() const { return terminado && ganado; }
bool NivelPenon::derrota() const { return terminado && !ganado; }

Capitan *NivelPenon::getCapitan() const { return capitan; }
Estructura *NivelPenon::getCanon() const { return canon; }
const Dificultad &NivelPenon::getDificultad() const { return dificultad; }
bool NivelPenon::estaBanderaPuesta() const { return banderaPuesta; }
float NivelPenon::getBanderaX() const { return BANDERA_X; }
float NivelPenon::getBanderaY() const { return BANDERA_Y; }
float NivelPenon::getBanderaRadio() const { return BANDERA_RADIO; }
