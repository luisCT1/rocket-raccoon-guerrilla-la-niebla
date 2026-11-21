#include "NivelEmboscada.h"
#include "Rocket.h"
#include "Enemigo.h"
#include "Estructura.h"
#include "MovOscilatorio.h"
#include "Eventos.h"

namespace {
//Mapa del nivel (px). Cambiar la disposición es cambiar estos números.
constexpr float ANCHO_MUNDO = 4000.f;
constexpr float ALTO_MUNDO = 720.f;
constexpr float ANCHO_VISTA = 1280.f;
//El piso queda más arriba para que el risco del camino se vea entero en la pantalla.
constexpr float SUELO = 548.f;
constexpr float HUECO_INI = 1500.f;       //barranco
constexpr float HUECO_FIN = 1920.f;
constexpr float PUENTE_X = 1590.f;        //el tablero deja 90 px de vacío a cada lado: hay que saltar
constexpr float PUENTE_AMPLITUD = 45.f;
constexpr float PUENTE_OMEGA = 1.6f;
constexpr float INICIO_X = 100.f;
const float CARRETA_X[3] = { 2400.f, 2900.f, 3400.f };
const float BARRIL_X[3] = { 2500.f, 3000.f, 3500.f }; //pegado a su carreta
const float SOLDADO_X[6] = { 2250.f, 2620.f, 2760.f, 3120.f, 3260.f, 3620.f };
constexpr float LIMITE_IZQ = 1940.f;      //los soldados no se meten al barranco
constexpr float LIMITE_DER = 3950.f;
constexpr float FIN_CAMINO = 3900.f;      //si el correo llega aquí, escapó
constexpr float RADIO_BARRIL = 110.f;
constexpr float DANIO_BARRIL = 120.f;      //más que la resistencia de una carreta
constexpr short MAX_SOLDADOS = 8;         //tope de soldados vivos
constexpr float ZONA_OLEADAS = 1300.f;    //las oleadas empiezan cuando Rocket se acerca al puente
constexpr float VISION_ALERTA = 700.f;
constexpr float SEGUIMIENTO_CAMARA = 5.f;

//Deja la entidad parada sobre el piso
void apoyarEnSuelo(Entidad *e) {
    e->setPosicion(e->getX(), SUELO - e->getAlto());
}
}

NivelEmboscada::NivelEmboscada()
    : posCamara(0.f),
      partesRestantes(3),
      correoEscapo(false),
      correoDetenido(false),
      intervaloOleada(6.f),
      tiempoOleada(0.f),
      puente(nullptr),
      carretas{nullptr, nullptr, nullptr},
      barriles{nullptr, nullptr, nullptr},
      correo(nullptr) {
    anchoMundo = ANCHO_MUNDO;
    altoMundo = ALTO_MUNDO;
    cenital = false;
    ySuelo = SUELO;
    huecoIni = HUECO_INI;
    huecoFin = HUECO_FIN;

    Rocket *r = new Rocket(INICIO_X, 0.f);
    apoyarEnSuelo(r);
    agregarEntidad(r);

    //Puente colgante: el modelo oscilatorio mueve su borde superior alrededor del nivel del piso
    puente = new Estructura(EST_PUENTE, PUENTE_X, SUELO);
    puente->agregarModelo(new MovOscilatorio(SUELO, PUENTE_AMPLITUD, PUENTE_OMEGA, 0.f, true, PUENTE_X));
    agregarEntidad(puente);
    registrarPlataforma(puente);

    for (short i = 0; i < 3; ++i) {
        carretas[i] = new Estructura(EST_CARRETA, CARRETA_X[i], 0.f);
        apoyarEnSuelo(carretas[i]);
        agregarEntidad(carretas[i]);

        barriles[i] = new Estructura(EST_BARRIL, BARRIL_X[i], 0.f);
        apoyarEnSuelo(barriles[i]);
        agregarEntidad(barriles[i]);
    }

    const float puestoX[] = { 2160.f, 2380.f, 2600.f, 2840.f, 3080.f, 3320.f, 3540.f, 3760.f };
    const short puestoAspecto[] = { 0, 2, 1, 4, 0, 3, 2, 0 };
    for (short i = 0; i < 8; ++i) {
        Enemigo *s = new Enemigo(puestoX[i], 0.f);
        apoyarEnSuelo(s);
        s->fijarAspecto(puestoAspecto[i]);
        s->fijarLimites(LIMITE_IZQ, LIMITE_DER);
        agregarEntidad(s);
    }
}

NivelEmboscada::~NivelEmboscada() {}

void NivelEmboscada::actualizar(float dt) {
    if (terminado) return;
    Nivel::actualizar(dt);

    //Barril destruido: explota y daña lo que tenga cerca (la carreta, soldados, otro barril)
    for (short i = 0; i < 3; ++i) {
        if (barriles[i] && barriles[i]->consumirDestruccion()) {
            emitirEvento(EV_EXPLOSION);
            explosionEn(barriles[i]->getCentroX(), barriles[i]->getCentroY(), RADIO_BARRIL, DANIO_BARRIL, true);
        }
    }
    for (short i = 0; i < 3; ++i) {
        if (carretas[i] && carretas[i]->consumirDestruccion()) {
            --partesRestantes;
            emitirEvento(EV_TAREA_COMPLETADA);
        }
    }

    if (partesRestantes <= 0 && !correo && !correoDetenido && !correoEscapo) soltarCorreo();

    revisarCorreo();

    if (rocket->getX() > ZONA_OLEADAS) {
        tiempoOleada += dt;
        if (tiempoOleada >= intervaloOleada) {
            tiempoOleada = 0.f;
            generarOleada();
        }
    }

    moverCamara(dt);

    if (rocket->getVidas() <= 0 || correoEscapo) terminar(false);
    else if (partesRestantes <= 0 && correoDetenido) terminar(true);
}

void NivelEmboscada::revisarCorreo() {
    if (!correo) return;
    if (!correo->estaViva()) {
        correoDetenido = true;
        correo = nullptr;
    } else if (correo->getX() >= FIN_CAMINO) {
        correoEscapo = true;
    }
}

//El Nivel va a borrar una entidad: si es el correo, se suelta el puntero antes
void NivelEmboscada::alBorrar(Entidad *e) {
    if (e == correo) {
        correoDetenido = true;
        correo = nullptr;
    }
    if (e == puente) puente = nullptr;
    for (short i = 0; i < 3; ++i) {
        if (carretas[i] == e) carretas[i] = nullptr;
        if (barriles[i] == e) barriles[i] = nullptr;
    }
    Nivel::alBorrar(e);
}

void NivelEmboscada::soltarCorreo() {
    correo = new Enemigo(CARRETA_X[2] + 60.f, 0.f);
    apoyarEnSuelo(correo);
    correo->fijarCorreo(true);
    correo->fijarAspecto(2);
    correo->fijarLimites(LIMITE_IZQ, LIMITE_DER);
    agregarEntidad(correo);
    emitirEvento(EV_CORREO_HUYE);
}

void NivelEmboscada::generarOleada() {
    if (contarVivos(CLASE_ENEMIGO) >= MAX_SOLDADOS) return;

    //Entra por el borde derecho de la pantalla, siempre del otro lado del barranco
    float sx = posCamara + ANCHO_VISTA + 40.f;
    if (sx < LIMITE_IZQ + 20.f) sx = LIMITE_IZQ + 20.f;
    if (sx > LIMITE_DER) sx = LIMITE_DER;

    Enemigo *s = new Enemigo(sx, 0.f);
    apoyarEnSuelo(s);
    s->fijarAspecto(static_cast<short>(int(sx / 90.f) % 5));
    s->fijarLimites(LIMITE_IZQ, LIMITE_DER);
    s->fijarAlcanceVision(VISION_ALERTA);
    agregarEntidad(s);
}

void NivelEmboscada::moverCamara(float dt) {
    //La cámara sigue a Rocket sin pegarse del todo. Depende de dt, no de los cuadros por segundo.
    const float objetivo = rocket->getX() - 400.f;
    float k = SEGUIMIENTO_CAMARA * dt;
    if (k > 1.f) k = 1.f;
    posCamara += (objetivo - posCamara) * k;
    if (posCamara < 0.f) posCamara = 0.f;
    else if (posCamara > ANCHO_MUNDO - ANCHO_VISTA) posCamara = ANCHO_MUNDO - ANCHO_VISTA;
    camaraX = posCamara;
}

bool NivelEmboscada::victoria() const { return terminado && ganado; }
bool NivelEmboscada::derrota() const { return terminado && !ganado; }

short NivelEmboscada::getPartesRestantes() const { return partesRestantes; }
bool NivelEmboscada::correoEnFuga() const { return correo != nullptr; }
bool NivelEmboscada::getCorreoEscapo() const { return correoEscapo; }
float NivelEmboscada::getHuecoInicio() const { return HUECO_INI; }
float NivelEmboscada::getHuecoFin() const { return HUECO_FIN; }
float NivelEmboscada::getFinCamino() const { return FIN_CAMINO; }
Estructura *NivelEmboscada::getPuente() const { return puente; }
