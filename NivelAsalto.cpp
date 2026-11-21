#include "NivelAsalto.h"
#include "Rocket.h"
#include "Enemigo.h"
#include "Estructura.h"
#include "Proyectil.h"
#include "Eventos.h"

namespace {
constexpr float ANCHO_MUNDO = 1600.f;
constexpr float ALTO_MUNDO = 900.f;
constexpr float TIEMPO_LIMITE = 150.f;
constexpr float CAMPAMENTO_X0 = 262.f, CAMPAMENTO_X1 = 1352.f;
constexpr float CAMPAMENTO_Y0 = 285.f, CAMPAMENTO_Y1 = 704.f;
constexpr float INICIO_X = 292.f;
constexpr float INICIO_Y = 618.f;
constexpr float SALIDA_X = 316.f;
constexpr float SALIDA_Y = 650.f;
constexpr float SALIDA_RADIO = 70.f;
constexpr float POLVORIN_X = 943.f, POLVORIN_Y = 321.f;
constexpr float BANDERA_X = 794.f, BANDERA_Y = 287.f;
constexpr float CANON_X = 1244.f, CANON_Y = 450.f;
const float TORRE_X[2] = { 565.f, 1033.f };
const float TORRE_Y[2] = { 431.f, 536.f };
const float TORRE_OMEGA[2] = { 0.9f, -0.9f };

constexpr float ALCANCE_CANON = 500.f;
constexpr float ENFRIAMIENTO_ALARMA = 6.f;
constexpr short REFUERZOS_POR_OLEADA = 2;
constexpr short MAX_SOLDADOS = 6;
constexpr float VISION_ALERTA = 2500.f;
constexpr short NUM_PUNTOS = 4;
const float PUNTO_X[NUM_PUNTOS] = { 850.f, 1273.f, 400.f, 1100.f };
const float PUNTO_Y[NUM_PUNTOS] = { 262.f, 364.f, 330.f, 632.f };
}

NivelAsalto::NivelAsalto()
    : tiempoLimite(TIEMPO_LIMITE),
      tiempoRestante(TIEMPO_LIMITE),
      tareasHechas(0),
      polvorinVolado(false),
      banderaTumbada(false),
      canonInutilizado(false),
      polvorin(nullptr),
      bandera(nullptr),
      canon(nullptr),
      enfriamientoAlarma(0.f),
      siguientePunto(0) {
    anchoMundo = ANCHO_MUNDO;
    altoMundo = ALTO_MUNDO;
    cenital = true;

    agregarEntidad(new Rocket(INICIO_X, INICIO_Y));

    polvorin = new Estructura(EST_POLVORIN, POLVORIN_X, POLVORIN_Y);
    agregarEntidad(polvorin);
    bandera = new Estructura(EST_BANDERA, BANDERA_X, BANDERA_Y);
    agregarEntidad(bandera);
    canon = new Estructura(EST_CANON, CANON_X, CANON_Y);
    agregarEntidad(canon);

    torres.reserve(2);
    for (short i = 0; i < 2; ++i) {
        Estructura *t = new Estructura(EST_TORRE, TORRE_X[i], TORRE_Y[i]);
        t->fijarVelocidadGiro(TORRE_OMEGA[i]);
        agregarEntidad(t);
        torres.push_back(t);
    }
}

NivelAsalto::~NivelAsalto() {}

void NivelAsalto::actualizar(float dt) {
    if (terminado) return;
    Nivel::actualizar(dt);
    mantenerEnElCampamento();

    actualizarReloj(dt);
    if (terminado) return;

    revisarTareas();

    if (canon->listaParaDisparar()) {
        const float dx = rocket->getCentroX() - canon->getCentroX();
        const float dy = rocket->getCentroY() - canon->getCentroY();
        if (dx * dx + dy * dy <= ALCANCE_CANON * ALCANCE_CANON) {
            Proyectil *p = canon->disparar(rocket->getCentroX(), rocket->getCentroY());
            if (p) {
                agregarEntidad(p);
                emitirEvento(EV_CANON_DISPARA);
            }
        }
    }

    if (enfriamientoAlarma > 0.f) enfriamientoAlarma -= dt;
    if (enfriamientoAlarma <= 0.f) {
        for (Estructura *t : torres) {
            if (t->detecta(rocket)) {
                emitirEvento(EV_ALARMA);
                generarRefuerzos();
                enfriamientoAlarma = ENFRIAMIENTO_ALARMA;
                break;
            }
        }
    }

    if (rocket->getVidas() <= 0) terminar(false);
    else if (tareasCompletas() && rocketEnSalida()) terminar(true);
}

void NivelAsalto::mantenerEnElCampamento() {
    float x = rocket->getX();
    float y = rocket->getY();
    const float xMax = CAMPAMENTO_X1 - rocket->getAncho();
    const float yMax = CAMPAMENTO_Y1 - rocket->getAlto();
    if (x < CAMPAMENTO_X0) x = CAMPAMENTO_X0;
    else if (x > xMax) x = xMax;
    if (y < CAMPAMENTO_Y0) y = CAMPAMENTO_Y0;
    else if (y > yMax) y = yMax;
    if (x != rocket->getX() || y != rocket->getY()) rocket->setPosicion(x, y);
}

void NivelAsalto::actualizarReloj(float dt) {
    if (terminado) return;
    tiempoRestante -= dt;
    if (tiempoRestante <= 0.f) {
        tiempoRestante = 0.f;
        terminar(false);
    }
}

void NivelAsalto::revisarTareas() {
    const short antes = tareasHechas;
    if (!polvorinVolado && polvorin->estaDestruida()) {
        polvorinVolado = true;
        ++tareasHechas;
        emitirEvento(EV_EXPLOSION);
    }
    if (!banderaTumbada && bandera->estaDestruida()) {
        banderaTumbada = true;
        ++tareasHechas;
    }
    if (!canonInutilizado && canon->estaDestruida()) {
        canonInutilizado = true;
        ++tareasHechas;
        emitirEvento(EV_EXPLOSION);
    }
    for (short i = antes; i < tareasHechas; ++i) {
        emitirEvento(EV_TAREA_COMPLETADA);
        generarRefuerzos();
    }
}

void NivelAsalto::generarRefuerzos() {
    short vivos = contarVivos(CLASE_ENEMIGO);
    for (short i = 0; i < REFUERZOS_POR_OLEADA && vivos < MAX_SOLDADOS; ++i, ++vivos) {
        Enemigo *s = new Enemigo(PUNTO_X[siguientePunto], PUNTO_Y[siguientePunto]);
        s->fijarAlcanceVision(VISION_ALERTA);
        agregarEntidad(s);
        siguientePunto = static_cast<short>((siguientePunto + 1) % NUM_PUNTOS);
    }
}

bool NivelAsalto::tareasCompletas() const {
    return polvorinVolado && banderaTumbada && canonInutilizado;
}

bool NivelAsalto::rocketEnSalida() const {
    const float dx = rocket->getCentroX() - SALIDA_X;
    const float dy = rocket->getCentroY() - SALIDA_Y;
    return dx * dx + dy * dy <= SALIDA_RADIO * SALIDA_RADIO;
}

bool NivelAsalto::victoria() const { return terminado && ganado; }
bool NivelAsalto::derrota() const { return terminado && !ganado; }

float NivelAsalto::getTiempoRestante() const { return tiempoRestante; }
float NivelAsalto::getTiempoLimite() const { return tiempoLimite; }
short NivelAsalto::getTareasHechas() const { return tareasHechas; }
bool NivelAsalto::estaPolvorinVolado() const { return polvorinVolado; }
bool NivelAsalto::estaBanderaTumbada() const { return banderaTumbada; }
bool NivelAsalto::estaCanonInutilizado() const { return canonInutilizado; }
float NivelAsalto::getSalidaX() const { return SALIDA_X; }
float NivelAsalto::getSalidaY() const { return SALIDA_Y; }
float NivelAsalto::getSalidaRadio() const { return SALIDA_RADIO; }
Estructura *NivelAsalto::getPolvorin() const { return polvorin; }
Estructura *NivelAsalto::getBandera() const { return bandera; }
Estructura *NivelAsalto::getCanon() const { return canon; }
const std::vector<Estructura *> &NivelAsalto::getTorres() const { return torres; }
