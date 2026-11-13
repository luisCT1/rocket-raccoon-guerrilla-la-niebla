#include "NivelAsalto.h"
#include "Estructura.h"
#include "Enemigo.h"
#include "Proyectil.h"
#include "Rocket.h"
#include "Eventos.h"

static const float ANCHO_MUNDO = 1600.0f;
static const float ALTO_MUNDO  = 1200.0f;
static const float TIEMPO_LIMITE = 120.0f;
static const float INICIO_X = 150.0f, INICIO_Y = 1050.0f;
static const float SALIDA_RADIO = 70.0f;
static const float POLVORIN_X = 1300.0f, POLVORIN_Y = 300.0f;
static const float BANDERA_X  = 800.0f,  BANDERA_Y  = 250.0f;
static const float CANON_X    = 1250.0f, CANON_Y    = 900.0f;
static const float TORRE_X[2] = { 500.0f, 1050.0f };
static const float TORRE_Y[2] = { 600.0f, 650.0f };
static const float TORRE_OMEGA[2] = { 0.9f, -0.9f };

static const float ALCANCE_CANON = 500.0f;
static const float ENFRIAMIENTO_ALARMA = 6.0f;
static const short REFUERZOS_POR_OLEADA = 2;
static const short MAX_SOLDADOS = 10;
static const short NUM_PUNTOS = 4;
static const float PUNTO_X[NUM_PUNTOS] = { 800.0f, 1540.0f, 60.0f, 1540.0f };
static const float PUNTO_Y[NUM_PUNTOS] = { 60.0f,  600.0f,  300.0f, 1100.0f };

NivelAsalto::NivelAsalto()
    : tiempoLimite(TIEMPO_LIMITE), tiempoRestante(TIEMPO_LIMITE), tareasHechas(0),
      polvorinVolado(false), banderaTumbada(false), canonInutilizado(false),
      polvorin(nullptr), bandera(nullptr), canon(nullptr), enfriamientoAlarma(0.0f), siguientePunto(0)
{
    anchoMundo = ANCHO_MUNDO;
    altoMundo  = ALTO_MUNDO;

    Rocket *r = new Rocket(INICIO_X, INICIO_Y);
    agregarEntidad(r);
    fijarJugador(r);

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

void NivelAsalto::actualizar(float dt)
{
    if (terminado) return;

    Nivel::actualizar(dt);
    actualizarReloj(dt);
    if (terminado) return;

    revisarTareas();

    const Entidad *jugador = rocket;
    if (canon->listaParaDisparar()) {
        float dx = jugador->getX() - canon->getX();
        float dy = jugador->getY() - canon->getY();
        if (dx * dx + dy * dy <= ALCANCE_CANON * ALCANCE_CANON) {
            Proyectil *p = canon->disparar(jugador->getX(), jugador->getY());
            if (p != nullptr) {
                agregarEntidad(p);
                emitirEvento(EV_CANON_DISPARA);
            }
        }
    }

    if (enfriamientoAlarma > 0.0f) enfriamientoAlarma -= dt;
    if (enfriamientoAlarma <= 0.0f) {
        for (Estructura *t : torres) {
            if (t->detecta(rocket)) {
                emitirEvento(EV_ALARMA);
                generarRefuerzos();
                enfriamientoAlarma = ENFRIAMIENTO_ALARMA;
                break;
            }
        }
    }

    if (rocket->getVidas() <= 0)                    terminar(false);
    else if (tareasCompletas() && rocketEnSalida()) terminar(true);
}

void NivelAsalto::actualizarReloj(float dt)
{
    if (terminado) return;
    tiempoRestante -= dt;
    if (tiempoRestante <= 0.0f) {
        tiempoRestante = 0.0f;
        terminar(false);
    }
}

void NivelAsalto::revisarTareas()
{
    short antes = tareasHechas;
    if (!polvorinVolado && polvorin->estaDestruida()) {
        polvorinVolado = true; ++tareasHechas;
        emitirEvento(EV_EXPLOSION);
    }
    if (!banderaTumbada && bandera->estaDestruida()) {
        banderaTumbada = true; ++tareasHechas;
    }
    if (!canonInutilizado && canon->estaDestruida()) {
        canonInutilizado = true; ++tareasHechas;
        emitirEvento(EV_EXPLOSION);
    }
    for (short i = antes; i < tareasHechas; ++i) {
        emitirEvento(EV_TAREA_COMPLETADA);
        generarRefuerzos();
    }
}

void NivelAsalto::generarRefuerzos()
{
    short vivos = 0;
    for (const Entidad *e : entidades)
        if (e->clase() == CLASE_ENEMIGO && e->estaViva()) ++vivos;

    for (short i = 0; i < REFUERZOS_POR_OLEADA && vivos < MAX_SOLDADOS; ++i, ++vivos) {
        Enemigo *s = new Enemigo(PUNTO_X[siguientePunto], PUNTO_Y[siguientePunto]);
        s->fijarObjetivo(rocket);
        agregarEntidad(s);
        siguientePunto = static_cast<short>((siguientePunto + 1) % NUM_PUNTOS);
    }
}

bool NivelAsalto::rocketEnSalida() const
{
    const Entidad *jugador = rocket;
    float dx = jugador->getX() - INICIO_X;
    float dy = jugador->getY() - INICIO_Y;
    return dx * dx + dy * dy <= SALIDA_RADIO * SALIDA_RADIO;
}

float NivelAsalto::getSalidaX() const { return INICIO_X; }
float NivelAsalto::getSalidaY() const { return INICIO_Y; }
float NivelAsalto::getSalidaRadio() const { return SALIDA_RADIO; }
