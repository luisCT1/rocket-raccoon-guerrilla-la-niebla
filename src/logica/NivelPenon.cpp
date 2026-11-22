#include "NivelPenon.h"
#include "Capitan.h"
#include "Estructura.h"
#include "Proyectil.h"
#include "Rocket.h"
#include "MovOscilatorio.h"
#include "AgenteTipos.h"
#include "Eventos.h"

namespace {
constexpr float ANCHO_MUNDO = 3920.f;
constexpr float ALTO_MUNDO = 720.f;
constexpr float ANCHO_VISTA = 1280.f;
constexpr float SUELO = 548.f;
constexpr float ROCKET_X = 150.f;
constexpr float HUECO_INI = 1980.f;
constexpr float HUECO_FIN = 2480.f;
constexpr float CAPITAN_X = 3140.f;
constexpr float CANON_X = 3520.f;
constexpr float LIMITE_IZQ = 2780.f;
constexpr float LIMITE_DER = 3400.f;
constexpr float CIMA_X = 2420.f;
constexpr float CIMA_Y = 388.f;
const float LATERAL_X[2] = { 2050.f, 2240.f };
const float LATERAL_Y[2] = { 488.f, 438.f };
const float LATERAL_AMPLITUD[2] = { 16.f, 16.f };
const float LATERAL_OMEGA[2] = { 1.35f, 1.65f };
constexpr float BANDERA_X = 2500.f;
constexpr float BANDERA_Y = 356.f;
constexpr float BANDERA_RADIO = 70.f;
constexpr float SEGUIMIENTO_CAMARA = 5.f;
}

NivelPenon::NivelPenon(short nivelDificultad, Capitan::Memoria *memoria)
    : capitan(nullptr), dificultad(nivelDificultad), banderaPuesta(false), canon(nullptr), canonAvisado(false) {
    anchoMundo = ANCHO_MUNDO;
    altoMundo = ALTO_MUNDO;
    cenital = false;
    ySuelo = SUELO;
    huecoIni = HUECO_INI;
    huecoFin = HUECO_FIN;
    disparosCanon.reserve(8);

    Rocket *r = new Rocket(ROCKET_X, 0.f);
    r->setPosicion(ROCKET_X, SUELO - r->getAlto());
    agregarEntidad(r);

    Estructura *cima = new Estructura(EST_PLATAFORMA, CIMA_X, CIMA_Y);
    agregarEntidad(cima);
    registrarPlataforma(cima);

    for (short i = 0; i < 2; ++i) {
        Estructura *p = new Estructura(EST_PLATAFORMA, LATERAL_X[i], LATERAL_Y[i]);
        p->agregarModelo(new MovOscilatorio(LATERAL_Y[i], LATERAL_AMPLITUD[i], LATERAL_OMEGA[i], i == 0 ? 0.f : PI_F, true, LATERAL_X[i]));
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
    capitan->fijarDificultad(dificultad);
}

NivelPenon::~NivelPenon() {}

void NivelPenon::actualizar(float dt) {
    if (terminado) return;
    Nivel::actualizar(dt);
    moverCamara(dt);

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

void NivelPenon::moverCamara(float dt) {
    if (!rocket) return;
    const float objetivo = rocket->getX() - 420.f;
    float k = SEGUIMIENTO_CAMARA * dt;
    if (k > 1.f) k = 1.f;
    camaraX += (objetivo - camaraX) * k;
    if (camaraX < 0.f) camaraX = 0.f;
    else if (camaraX > anchoMundo - ANCHO_VISTA) camaraX = anchoMundo - ANCHO_VISTA;
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
short NivelPenon::getDificultad() const { return dificultad; }
bool NivelPenon::estaBanderaPuesta() const { return banderaPuesta; }
float NivelPenon::getBanderaX() const { return BANDERA_X; }
float NivelPenon::getBanderaY() const { return BANDERA_Y; }
float NivelPenon::getBanderaRadio() const { return BANDERA_RADIO; }
