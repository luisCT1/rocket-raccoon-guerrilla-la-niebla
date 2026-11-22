#include "NivelPenon.h"
#include "Capitan.h"
#include "Estructura.h"
#include "Proyectil.h"
#include "Recogible.h"
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
    : capitan(nullptr), dificultad(nivelDificultad), banderaPuesta(false), canon(nullptr), canonAvisado(false), vozInicio(false) {
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

    const float recoX[] = { 520.f, 980.f, 1500.f, 1780.f, CIMA_X + 50.f, 2920.f, 3360.f };
    const float recoY[] = { SUELO - 186.f, SUELO - 186.f, SUELO - 186.f, SUELO - 186.f, CIMA_Y - 186.f, SUELO - 186.f, SUELO - 186.f };
    const short recoT[] = { RECO_MUNICION, RECO_GRANADA, RECO_FURIA, RECO_VIDA, RECO_MUNICION, RECO_GRANADA, RECO_FURIA };
    for (short i = 0; i < 7; ++i)
        agregarEntidad(new Recogible(recoT[i], recoX[i], recoY[i]));
}

NivelPenon::~NivelPenon() {}

void NivelPenon::actualizar(float dt) {
    if (terminado) return;
    if (!vozInicio) {
        vozInicio = true;
        emitirEvento(EV_VOZ_RINDE);
    }
    Nivel::actualizar(dt);
    if (capitan && capitan->estaPersiguiendo())
        capitan->fijarLimites(40.f, anchoMundo - capitan->getAncho() - 20.f);
    seguirCapitan();
    moverCamara(dt);

    if (!canonAvisado && canon->estaDestruida()) {
        canonAvisado = true;
        capitan->fijarCanonDisponible(false);
        emitirEvento(EV_EXPLOSION);
    }

    const short accion = capitan->consumirInicioAccion();
    if (accion == ACC_GIRATORIO) emitirEvento(EV_ATAQUE_GIRATORIO);
    else if (accion == ACC_TROMBA) emitirEvento(EV_TROMBA);
    else if (accion == ACC_ACOSAR && capitan->getFase() == FASE_ACOSA) emitirEvento(EV_VOZ_NO_ESCAPARAS);
    if (capitan->consumirSable()) emitirEvento(EV_SABLE);

    float objX, objY;
    if (capitan->consumirOrdenCanon(objX, objY)) {
        Proyectil *p = canon->disparar(objX, objY);
        if (p) {
            agregarEntidad(p);
            disparosCanon.push_back(p);
            emitirEvento(EV_CANON_DISPARA);
            if (capitan->getFase() == FASE_CANON) emitirEvento(EV_VOZ_FUEGO);
        }
    }
    seguirDisparos();

    if (capitan->alcanzaA(rocket) && rocket->recibirDanio()) capitan->confirmarGolpe();
    notificarGolpe();
    if (capitan->consumirImpacto()) {
        if (capitan->estaDerrotado()) emitirEvento(EV_VOZ_IMPOSIBLE);
        else emitirEvento(EV_VOZ_ALIMANA);
    }

    if (capitan->consumirCambioFase()) cambiarFase();

    if (rocket->getVidas() <= 0) {
        terminar(false);
    } else if (capitan->estaDerrotado()) {
        const float dx = rocket->getCentroX() - BANDERA_X;
        const float dy = rocket->getCentroY() - BANDERA_Y;
        if (dx * dx + dy * dy <= BANDERA_RADIO * BANDERA_RADIO) ponerBandera();
    }
}

void NivelPenon::seguirCapitan() {
    if (!capitan) return;

    struct Tramo { float x0, x1, y; };
    Tramo tramos[8];
    int n = 0;
    tramos[n++] = { 0.f, huecoIni, ySuelo };
    tramos[n++] = { huecoFin, anchoMundo, ySuelo };
    for (Entidad *p : plataformas) {
        if (!p || n >= 8) continue;
        tramos[n++] = { p->getX(), p->getX() + p->getAncho(), p->getY() };
    }

    const float cx = capitan->getCentroX();
    const float pies = capitan->getY() + capitan->getAlto();
    int bajo = -1;
    float mejor = 1.0e9f;
    for (int i = 0; i < n; ++i) {
        if (cx < tramos[i].x0 || cx > tramos[i].x1) continue;
        float dy = pies - tramos[i].y;
        if (dy < 0.f) dy = -dy;
        if (dy < mejor) {
            mejor = dy;
            bajo = i;
        }
    }
    if (bajo >= 0) {
        capitan->setPosicion(capitan->getX(), tramos[bajo].y - capitan->getAlto());
        return;
    }

    int cerca = -1;
    float dist = 1.0e9f;
    float borde = cx;
    const float firma = rocket ? rocket->getCentroX() - cx : 0.f;
    for (int i = 0; i < n; ++i) {
        float d;
        float b;
        if (cx < tramos[i].x0) {
            d = tramos[i].x0 - cx;
            b = tramos[i].x0;
        } else if (cx > tramos[i].x1) {
            d = cx - tramos[i].x1;
            b = tramos[i].x1;
        } else {
            d = 0.f;
            b = cx;
        }
        if ((b - cx) * firma < 0.f && d > 8.f) d += 40.f;
        if (d < dist) {
            dist = d;
            cerca = i;
            borde = b;
        }
    }
    if (cerca < 0) return;
    if (dist <= 110.f)
        capitan->setPosicion(capitan->getX(), tramos[cerca].y - capitan->getAlto());
    else
        capitan->setPosicion(borde - capitan->getAncho() * 0.5f, tramos[cerca].y - capitan->getAlto());
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
    if (capitan && capitan->getFase() == FASE_AGRESIVO) emitirEvento(EV_VOZ_CARGA);
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
