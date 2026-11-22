#include "NivelAsalto.h"
#include "Rocket.h"
#include "Enemigo.h"
#include "Estructura.h"
#include "Proyectil.h"
#include "Eventos.h"

namespace {
constexpr float ANCHO_MUNDO = 1600.f;
constexpr float ALTO_MUNDO = 900.f;
constexpr float TIEMPO_LIMITE = 120.f;
constexpr float CAMPAMENTO_X0 = 262.f, CAMPAMENTO_X1 = 1352.f;
constexpr float CAMPAMENTO_Y0 = 285.f, CAMPAMENTO_Y1 = 704.f;
constexpr float INICIO_X = 292.f;
constexpr float INICIO_Y = 618.f;
constexpr float POLVORIN_X = 943.f, POLVORIN_Y = 321.f;
constexpr float BANDERA_X = 794.f, BANDERA_Y = 287.f;
constexpr float CANON_X = 1244.f, CANON_Y = 450.f;
const float TORRE_X[2] = { 565.f, 1033.f };
const float TORRE_Y[2] = { 431.f, 536.f };
const float TORRE_OMEGA[2] = { 0.9f, -0.9f };

constexpr short NUM_BARRILES = 3;
const float BARRIL_X[NUM_BARRILES] = { 430.f, 462.f, 700.f };
const float BARRIL_Y[NUM_BARRILES] = { 470.f, 492.f, 540.f };
constexpr float RADIO_BARRIL = 85.f;
constexpr float DANIO_BARRIL = 55.f;
constexpr float DANIO_BARRIL_ROCKET = 35.f;
constexpr short MAX_PASADAS_BARRIL = 8;

constexpr short NUM_BARRERAS = 4;
constexpr float BARRERA_ANCHO = 120.f;
constexpr float BARRERA_ALTO = 22.f;
const float BARRERA_X[NUM_BARRERAS] = { 1200.f, 1200.f, 1090.f, 1090.f };
const float BARRERA_Y[NUM_BARRERAS] = { 408.f, 522.f, 412.f, 504.f };

constexpr float RESISTENCIA_CANON = 450.f;
constexpr float ENFRIAMIENTO_ALARMA = 6.f;
constexpr float TIEMPO_CALMA = 18.f;
constexpr float CADENCIA_CANON[4] = { 1.8f, 1.6f, 1.3f, 1.0f };
constexpr float MULT_CONO[4] = { 1.f, 1.1f, 1.25f, 1.4f };
constexpr float MULT_GIRO[4] = { 1.f, 1.15f, 1.3f, 1.6f };

constexpr short NUM_PUNTOS = 4;
const float PUNTO_X[NUM_PUNTOS] = { 850.f, 1273.f, 480.f, 1100.f };
const float PUNTO_Y[NUM_PUNTOS] = { 262.f, 330.f, 290.f, 632.f };
constexpr short REFUERZOS_POR_ALARMA = 1;
constexpr short MAX_SOLDADOS = 6;
constexpr short MAX_SOLDADOS_ALARMA_MAX = 8;
constexpr short MAX_ABSOLUTO = 12;
constexpr float VISION_PATRULLA = 320.f;
constexpr float VISION_ALERTA = 2500.f;

constexpr short ESTRUCTURAS_PARA_OLEADA = 2;
constexpr short SOLDADOS_POR_OLEADA = 4;
constexpr short NUM_PUERTAS = 5;
const float PUERTA_X[NUM_PUERTAS] = { 750.f, 795.f, 840.f, 885.f, 705.f };
constexpr float PUERTA_Y = 700.f;
constexpr float AVISO_REFUERZOS = 4.f;
constexpr float TIEMPO_RESISTENCIA = 20.f;
constexpr float INTERVALO_RESISTENCIA = 1.6f;
}

NivelAsalto::NivelAsalto()
    : tiempoRestante(TIEMPO_LIMITE),
      polvorin(nullptr),
      bandera(nullptr),
      canon(nullptr),
      enfriamientoAlarma(0.f),
      sinDeteccion(0.f),
      radioConoBase(0.f),
      aperturaConoBase(0.f),
      avisoRefuerzos(0.f),
      avisoResistencia(0.f),
      proximoSoldado(0.f),
      siguientePunto(0),
      siguientePuerta(0),
      nivelAlarma(0),
      coronelLlego(false),
      resistencia(false) {
    anchoMundo = ANCHO_MUNDO;
    altoMundo = ALTO_MUNDO;
    cenital = true;

    agregarEntidad(new Rocket(INICIO_X, INICIO_Y));

    polvorin = new Estructura(EST_POLVORIN, POLVORIN_X, POLVORIN_Y);
    agregarEntidad(polvorin);
    bandera = new Estructura(EST_BANDERA, BANDERA_X, BANDERA_Y);
    agregarEntidad(bandera);
    canon = new Estructura(EST_CANON, CANON_X, CANON_Y);
    canon->fijarResistencia(RESISTENCIA_CANON);
    canon->fijarCadencia(CADENCIA_CANON[0]);
    agregarEntidad(canon);

    torres.reserve(2);
    for (short i = 0; i < 2; ++i) {
        Estructura *t = new Estructura(EST_TORRE, TORRE_X[i], TORRE_Y[i]);
        t->fijarVelocidadGiro(TORRE_OMEGA[i]);
        agregarEntidad(t);
        torres.push_back(t);
    }
    radioConoBase = torres[0]->getRadioCono();
    aperturaConoBase = torres[0]->getApertura();

    barriles.reserve(NUM_BARRILES);
    for (short i = 0; i < NUM_BARRILES; ++i) {
        Estructura *b = new Estructura(EST_BARRIL, BARRIL_X[i], BARRIL_Y[i]);
        agregarEntidad(b);
        barriles.push_back(b);
    }

    for (short i = 0; i < NUM_BARRERAS; ++i) {
        Estructura *b = new Estructura(EST_PLATAFORMA, BARRERA_X[i], BARRERA_Y[i]);
        b->fijarTamano(BARRERA_ANCHO, BARRERA_ALTO);
        agregarEntidad(b);
    }

    for (short i = 0; i < NUM_PUNTOS; ++i) agregarEntidad(crearSoldado(PUNTO_X[i], PUNTO_Y[i]));
}

void NivelAsalto::actualizar(float dt) {
    if (terminado) return;
    Nivel::actualizar(dt);
    mantenerEnElCampamento();

    tiempoRestante -= dt;
    if (tiempoRestante < 0.f) tiempoRestante = 0.f;

    revisarTareas();
    procesarBarriles();
    disparoDelCanon();
    vigilar(dt);
    if (avisoRefuerzos > 0.f) avisoRefuerzos -= dt;
    if (avisoResistencia > 0.f) avisoResistencia -= dt;
    if (resistencia) {
        proximoSoldado -= dt;
        if (proximoSoldado <= 0.f) {
            proximoSoldado += INTERVALO_RESISTENCIA;
            soltarSoldados(1);
        }
    }

    if (rocket->getVidas() <= 0) terminar(false);
    else if (tiempoRestante <= 0.f) terminar(resistencia);
}

void NivelAsalto::disparoDelCanon() {
    if (!canon->listaParaDisparar()) return;
    Proyectil *p = canon->disparar(rocket->getCentroX(), rocket->getCentroY());
    if (p) {
        agregarEntidad(p);
        emitirEvento(EV_CANON_DISPARA);
    }
}

void NivelAsalto::vigilar(float dt) {
    if (enfriamientoAlarma > 0.f) enfriamientoAlarma -= dt;

    bool visto = false;
    for (const Estructura *t : torres) {
        if (t->detecta(rocket)) {
            visto = true;
            break;
        }
    }
    if (!visto) {
        for (const Entidad *e : entidades) {
            if (e->clase() != CLASE_ENEMIGO || !e->estaViva()) continue;
            if (static_cast<const Enemigo *>(e)->veA(rocket)) {
                visto = true;
                break;
            }
        }
    }

    if (visto) {
        sinDeteccion = 0.f;
        if (enfriamientoAlarma <= 0.f) subirAlarma();
    } else if (nivelAlarma > 0) {
        sinDeteccion += dt;
        if (sinDeteccion >= TIEMPO_CALMA) {
            sinDeteccion = 0.f;
            --nivelAlarma;
            aplicarNivelAlarma();
        }
    }
}

void NivelAsalto::subirAlarma() {
    if (nivelAlarma < 3) {
        ++nivelAlarma;
        emitirEvento(EV_ALARMA);
    }
    generarRefuerzos();
    enfriamientoAlarma = ENFRIAMIENTO_ALARMA;
    aplicarNivelAlarma();
}

void NivelAsalto::aplicarNivelAlarma() {
    const float radio = radioConoBase * MULT_CONO[nivelAlarma];
    const float giro = MULT_GIRO[nivelAlarma];
    for (short i = 0; i < static_cast<short>(torres.size()); ++i) {
        torres[i]->fijarCono(radio, aperturaConoBase);
        torres[i]->fijarVelocidadGiro(TORRE_OMEGA[i] * giro);
    }
    canon->fijarCadencia(CADENCIA_CANON[nivelAlarma]);
    const float vision = visionActual();
    for (Entidad *e : entidades) {
        if (e->clase() == CLASE_ENEMIGO) static_cast<Enemigo *>(e)->fijarAlcanceVision(vision);
    }
}

float NivelAsalto::visionActual() const {
    return nivelAlarma > 0 ? VISION_ALERTA : VISION_PATRULLA;
}

void NivelAsalto::procesarBarriles() {
    for (short pasada = 0; pasada < MAX_PASADAS_BARRIL; ++pasada) {
        bool exploto = false;
        for (Estructura *b : barriles) {
            if (!b->consumirDestruccion()) continue;
            exploto = true;
            explosionEn(b->getCentroX(), b->getCentroY(), RADIO_BARRIL, DANIO_BARRIL, true);
            explosionEn(b->getCentroX(), b->getCentroY(), RADIO_BARRIL, DANIO_BARRIL_ROCKET, false);
            emitirEvento(EV_BARRIL);
        }
        if (!exploto) break;
    }
}

void NivelAsalto::soltarSoldados(short cuantos) {
    short vivos = contarVivos(CLASE_ENEMIGO);
    for (short i = 0; i < cuantos && vivos < MAX_ABSOLUTO; ++i, ++vivos) {
        agregarEntidad(crearSoldado(PUERTA_X[siguientePuerta], PUERTA_Y));
        siguientePuerta = static_cast<short>((siguientePuerta + 1) % NUM_PUERTAS);
    }
}

void NivelAsalto::iniciarResistencia() {
    resistencia = true;
    tiempoRestante = TIEMPO_RESISTENCIA;
    proximoSoldado = INTERVALO_RESISTENCIA;
    nivelAlarma = 3;
    sinDeteccion = 0.f;
    aplicarNivelAlarma();
    emitirEvento(EV_ALARMA);
    soltarSoldados(SOLDADOS_POR_OLEADA);
    emitirEvento(EV_REFUERZOS);
    avisoResistencia = AVISO_REFUERZOS;
}

Enemigo *NivelAsalto::crearSoldado(float x, float y) {
    Enemigo *s = new Enemigo(x, y);
    s->fijarAlcanceVision(visionActual());
    return s;
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

void NivelAsalto::revisarTareas() {
    if (polvorin->consumirDestruccion()) {
        emitirEvento(EV_EXPLOSION);
        tareaCompletada();
    }
    if (bandera->consumirDestruccion()) tareaCompletada();
    if (canon->consumirDestruccion()) {
        emitirEvento(EV_EXPLOSION);
        tareaCompletada();
    }
    const short destruidas = static_cast<short>((polvorin->estaDestruida() ? 1 : 0) + (bandera->estaDestruida() ? 1 : 0) + (canon->estaDestruida() ? 1 : 0));
    if (!coronelLlego && destruidas >= ESTRUCTURAS_PARA_OLEADA) {
        coronelLlego = true;
        soltarSoldados(SOLDADOS_POR_OLEADA);
        emitirEvento(EV_REFUERZOS);
        avisoRefuerzos = AVISO_REFUERZOS;
    }
    if (!resistencia && tareasCompletas()) iniciarResistencia();
}

void NivelAsalto::tareaCompletada() {
    emitirEvento(EV_TAREA_COMPLETADA);
    generarRefuerzos();
}

void NivelAsalto::generarRefuerzos() {
    const short tope = nivelAlarma >= 3 ? MAX_SOLDADOS_ALARMA_MAX : MAX_SOLDADOS;
    short vivos = contarVivos(CLASE_ENEMIGO);
    for (short i = 0; i < REFUERZOS_POR_ALARMA && vivos < tope; ++i, ++vivos) {
        agregarEntidad(crearSoldado(PUNTO_X[siguientePunto], PUNTO_Y[siguientePunto]));
        siguientePunto = static_cast<short>((siguientePunto + 1) % NUM_PUNTOS);
    }
}

bool NivelAsalto::tareasCompletas() const {
    return polvorin->estaDestruida() && bandera->estaDestruida() && canon->estaDestruida();
}

bool NivelAsalto::victoria() const { return terminado && ganado; }
bool NivelAsalto::derrota() const { return terminado && !ganado; }

float NivelAsalto::getTiempoRestante() const { return tiempoRestante; }
bool NivelAsalto::estaEnResistencia() const { return resistencia; }
bool NivelAsalto::estaPolvorinVolado() const { return polvorin->estaDestruida(); }
bool NivelAsalto::estaBanderaTumbada() const { return bandera->estaDestruida(); }
bool NivelAsalto::estaCanonInutilizado() const { return canon->estaDestruida(); }
Estructura *NivelAsalto::getPolvorin() const { return polvorin; }
Estructura *NivelAsalto::getBandera() const { return bandera; }
Estructura *NivelAsalto::getCanon() const { return canon; }
const std::vector<Estructura *> &NivelAsalto::getTorres() const { return torres; }

short NivelAsalto::getNivelAlarma() const { return nivelAlarma; }
float NivelAsalto::getAvisoRefuerzos() const { return avisoRefuerzos > 0.f ? avisoRefuerzos : 0.f; }
float NivelAsalto::getAvisoResistencia() const { return avisoResistencia > 0.f ? avisoResistencia : 0.f; }
