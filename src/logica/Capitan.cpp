#include "Capitan.h"
#include "MovCircular.h"
#include "Rocket.h"
#include "Eventos.h"
#include <cmath>
#include <stdexcept>

namespace {
constexpr float ANCHO_CAPITAN = 60.f;
constexpr float ALTO_CAPITAN = 90.f;
constexpr float VIDA_CAPITAN = 1000.f;
constexpr float INTERVALO_DECISION = 0.2f;
constexpr float PAUSA_ENTRE_ACCIONES = 0.8f;
constexpr float VELOCIDAD = 140.f;
constexpr float RADIO_GIRO = 110.f;
constexpr float RADIO_GOLPE_GIRO = 35.f;
const float OMEGA_FASE[3] = { 4.f, 5.f, 6.5f };
constexpr float TROMBA_MAX = 260.f;
constexpr float TROMBA_GROSOR = 45.f;
constexpr float TROMBA_ALTURA = 80.f;

constexpr float ALCANCE_VISTA = 600.0f;
constexpr float UMBRAL_ALTO = 60.0f;
constexpr float INTENSIDAD_DASH = 0.8f;
constexpr float INTENSIDAD_GRANADA = 1.0f;
constexpr float VISTA_MIN = 0.1f;
constexpr float VISTA_MAX = 0.6f;
constexpr float TOLERANCIA = 1e-4f;

const float BASE[3][ACC_NUM] = {
      {  3.0f,   1.6f,    1.0f,   -1.0f,     0.5f },
     {  1.5f,   3.0f,    1.5f,    1.0f,     1.0f },
      {  2.5f,   2.0f,    3.0f,    3.0f,    -1.0f }
};
const float RECARGA[ACC_NUM] = { 0.5f, 2.0f, 4.0f, 6.0f, 3.0f };
constexpr float CERCA = 150.0f;
constexpr float LEJOS = 300.0f;
constexpr float EMPATE = 1e-4f;

const float DURACION[ACC_NUM] = { 1.2f, 0.6f, 2.0f, 1.5f, 1.0f };

constexpr float ESCALA_PESO = 1.5f;
constexpr float BONO_HABITO = 1.5f;
constexpr int CONFIANZA_MIN = 10;
constexpr short TOPE_CONTEO = 30000;
constexpr float TIEMPO_HABITO = 2.f;
constexpr float DISTANCIA_LEJOS = 300.f;
const short CONTRA[HAB_NUM] = {
    ACC_GIRATORIO,
    ACC_TROMBA,
    ACC_CANON,
    ACC_ACOSAR
};
}

Capitan::Memoria::Memoria() {
    conteoHabitos.fill(0);
    pesosAccion.fill(0.0f);
    bonoAccion.fill(0.0f);
}

Capitan::Capitan(float x_, float y_, Memoria *memoria_)
    : Enemigo(x_, y_),
      fase(FASE_ACOSA), vidaMax(VIDA_CAPITAN), acumulador(0.f), descanso(0.f),
      resultadoReportado(true), yaGolpeo(false),
      ordenCanon(false), canonX(0.f), canonY(0.f), cambioFase(false),
      sablePendiente(false), sableSonado(false), vozPendiente(false), vozEspera(0.f),
      inicioAccion(ACC_NINGUNA),
      giro(nullptr), factorVelocidad(1.f), derrotado(false), encontrado(false),
      estimuloVisible(ESTIM_NINGUNO), intensidadEstimulo(0.f), distanciaRocket(0.f), ladoRocket(1), rocketEnAlto(false),
      radioOido(400.f), retrasoReaccion(0.f), estimuloPendiente(ESTIM_NINGUNO), intensidadPendiente(0.f), esperaReaccion(0.f),
      agresividad(1.f), factorRecarga(1.f),
      idAccion(ACC_NINGUNA), accionRestante(0.f), accionDuracion(0.f), destinoX(0.f), destinoY(0.f),
      accionFinalizada(true), accionAcierto(false),
      memoriaPropia(), memoria(memoria_ ? memoria_ : &memoriaPropia),
      tasaAprendizaje(0.2f), estimuloPrevio(ESTIM_NINGUNO), tiempoAlto(0.f), tiempoLejos(0.f) {
    recargas.fill(0.f);
    sinUsar.fill(0.f);
    bloqueadas.fill(false);

    ancho = ANCHO_CAPITAN;
    alto = ALTO_CAPITAN;
    vida = VIDA_CAPITAN;
    seguirModelos = false;

    giro = new MovCircular(getCentroX(), getCentroY(), RADIO_GIRO, OMEGA_FASE[0]);
    giro->activar(false);
    agregarModelo(giro);
}

Capitan::~Capitan() {}

void Capitan::actualizar(float dt) {
    if (derrotado) {
        avanzo = false;
        return;
    }
    const float x0 = x;
    const float y0 = y;

    Entidad::actualizar(dt);
    actualizarRecargas(dt);
    actualizarAccion(dt);
    if (vozEspera > 0.f)
        vozEspera = vozEspera > dt ? vozEspera - dt : 0.f;

    if (accionFinalizada && !resultadoReportado) {
        finalizarAccion();
        descanso = PAUSA_ENTRE_ACCIONES / factorVelocidad;
    } else if (descanso > 0.f) {
        descanso -= dt;
    }

    acumulador += dt;
    const bool libre = accionFinalizada && descanso <= 0.f;
    if (acumulador >= INTERVALO_DECISION - 1.0e-4f || libre) {
        ciclo(objetivo, acumulador);
        acumulador = 0.f;
    }
    moverse(dt);
    notarDesplazamiento(x0, y0);
}

void Capitan::ciclo(const Rocket *r, float dt) {
    if (derrotado || !r) return;

    percibir(r, dt);
    observarHabitos(dt);

    if (!accionFinalizada) return;

    if (!resultadoReportado) {
        finalizarAccion();
        descanso = PAUSA_ENTRE_ACCIONES / factorVelocidad;
    }
    if (descanso > 0.f) return;

    const short a = decidir();
    if (a != ACC_NINGUNA) ejecutarAccion(a, r);
}

void Capitan::finalizarAccion() {
    giro->activar(false);
    const short a = idAccion;
    if (a == ACC_NINGUNA || resultadoReportado) return;
    resultadoReportado = true;
    if (a != ACC_CANON && a != ACC_REPLEGARSE)
        resultado(a, accionAcierto);
}

void Capitan::moverse(float dt) {
    if (!objetivo) return;

    const float dx = objetivo->getCentroX() - getCentroX();
    const float sentido = dx >= 0.f ? 1.f : -1.f;
    if (dx != 0.f) direccion = dx >= 0.f ? 'R' : 'L';
    const float dist = dx >= 0.f ? dx : -dx;
    const float vel = VELOCIDAD * factorVelocidad * dt;

    float paso = 0.f;
    if (!accionFinalizada && idAccion == ACC_REPLEGARSE) {
        paso = -sentido * vel;
    } else if (!accionFinalizada && (idAccion == ACC_ACOSAR || idAccion == ACC_GIRATORIO || idAccion == ACC_TROMBA)) {
        if (dist > 100.f) paso = sentido * vel;
    } else if (!accionFinalizada && idAccion == ACC_CANON) {
        if (dist < 280.f) paso = -sentido * vel * 0.65f;
        else if (dist > 520.f) paso = sentido * vel * 0.65f;
    } else if (encontrado && dist > 620.f) {
        paso = sentido * vel;
    }
    x += paso;
    if (x < minX) x = minX;
    else if (x > maxX) x = maxX;

    if (giro->estaActivo()) giro->fijarCentro(getCentroX(), getCentroY());
}

short Capitan::clase() const {
    return CLASE_CAPITAN;
}

short Capitan::estadoVisual() const {
    if (derrotado) return 99;
    return accionFinalizada ? 0 : static_cast<short>(idAccion + 1);
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
        pedirVoz();
        giro->fijarOmega(OMEGA_FASE[fase - 1] * factorVelocidad);
    }
}

bool Capitan::alcanzaA(const Rocket *r) const {
    if (derrotado || !r || yaGolpeo || accionFinalizada) return false;

    switch (idAccion) {
    case ACC_ACOSAR: {
        const float t = avanceAccion();
        if (t < 0.34f || t > 0.70f) return false;
        const float dir = direccion == 'L' ? -1.f : 1.f;
        const float sx = getCentroX() + dir * 42.f;
        const float sy = getCentroY() - 16.f;
        const float dx = r->getCentroX() - sx;
        const float dy = r->getCentroY() - sy;
        return dx * dx + dy * dy <= 74.f * 74.f;
    }
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
    accionAcierto = true;
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

bool Capitan::consumirSable() {
    const bool c = sablePendiente;
    sablePendiente = false;
    return c;
}

bool Capitan::consumirVoz() {
    const bool c = vozPendiente;
    vozPendiente = false;
    return c;
}

void Capitan::pedirVoz() {
    if (derrotado || vozEspera > 0.f) return;
    vozPendiente = true;
    vozEspera = 5.2f;
}

bool Capitan::muestraSable() const {
    if (derrotado || accionFinalizada) return false;
    if (idAccion == ACC_GIRATORIO) return true;
    if (idAccion != ACC_ACOSAR) return false;
    const float t = avanceAccion();
    return t >= 0.30f && t <= 0.74f;
}

short Capitan::consumirInicioAccion() {
    const short a = inicioAccion;
    inicioAccion = ACC_NINGUNA;
    return a;
}

void Capitan::resultadoCanon(bool acerto) {
    resultado(ACC_CANON, acerto);
}

void Capitan::fijarCanonDisponible(bool disponible) {
    bloquear(ACC_CANON, !disponible);
}

short Capitan::getFase() const { return fase; }
bool Capitan::estaDerrotado() const { return derrotado; }
bool Capitan::estaPersiguiendo() const { return encontrado && !derrotado; }
float Capitan::getVidaMax() const { return vidaMax; }

bool Capitan::giratorioActivo() const {
    return !derrotado && giro->estaActivo();
}

void Capitan::posicionGiratorio(float &gx, float &gy) const {
    giro->posicion(gx, gy);
}

static const float DIF_RETRASO[3]     = { 0.60f, 0.35f, 0.15f };
static const float DIF_TASA[3]        = { 0.10f, 0.20f, 0.35f };
static const float DIF_AGRESIVIDAD[3] = { 0.80f, 1.00f, 1.30f };
static const float DIF_VELOCIDAD[3]   = { 0.75f, 1.00f, 1.35f };

void Capitan::fijarDificultad(short nivel) {
    if (nivel < DIF_FACIL || nivel > DIF_DIFICIL) throw std::out_of_range("Capitan: dificultad desconocida");
    fijarRetrasoPercepcion(DIF_RETRASO[nivel]);
    fijarTasaAprendizaje(DIF_TASA[nivel]);
    fijarAgresividad(DIF_AGRESIVIDAD[nivel]);
    fijarVelocidadAtaques(DIF_VELOCIDAD[nivel]);
}

bool Capitan::trombaActiva() const {
    return !derrotado && !accionFinalizada && idAccion == ACC_TROMBA;
}

float Capitan::radioTromba() const {
    return trombaActiva() ? TROMBA_MAX * avanceAccion() : 0.f;
}

void Capitan::percibir(const Rocket *r, float dt) {
    if (r == nullptr) {
        estimuloVisible = ESTIM_NINGUNO;
        intensidadEstimulo = 0.f;
        estimuloPendiente = ESTIM_NINGUNO;
        return;
    }

    const float dx = r->getCentroX() - getCentroX();
    const float dy = r->getCentroY() - getCentroY();
    distanciaRocket = std::sqrt(dx * dx + dy * dy);
    ladoRocket = dx >= 0.f ? 1 : -1;
    rocketEnAlto = r->getCentroY() < getCentroY() - UMBRAL_ALTO;

    short tipo = ESTIM_NINGUNO;
    float inten = 0.f;

    if (distanciaRocket <= ALCANCE_VISTA) {
        const float v = 1.f - distanciaRocket / ALCANCE_VISTA;
        inten = v < VISTA_MIN ? VISTA_MIN : (v > VISTA_MAX ? VISTA_MAX : v);
        tipo = ESTIM_VISTA;
    }
    const float ruido = r->ruidoActual();
    if (ruido > 0.f && distanciaRocket <= ruido && distanciaRocket <= radioOido && INTENSIDAD_DASH > inten) {
        inten = INTENSIDAD_DASH;
        tipo = ESTIM_DASH;
    }
    if (r->lanzoGranadaReciente() && INTENSIDAD_GRANADA > inten) {
        inten = INTENSIDAD_GRANADA;
        tipo = ESTIM_GRANADA;
    }

    if (tipo != estimuloPendiente) {
        estimuloPendiente = tipo;
        esperaReaccion = retrasoReaccion;
    } else {
        esperaReaccion -= dt;
    }
    intensidadPendiente = inten;

    if (esperaReaccion <= TOLERANCIA) {
        estimuloVisible = estimuloPendiente;
        intensidadEstimulo = intensidadPendiente;
    }
    if (!encontrado && (estimuloVisible == ESTIM_VISTA || distanciaRocket <= 90.f)) {
        encontrado = true;
        pedirVoz();
    }
}

short Capitan::estimulo() const { return estimuloVisible; }
float Capitan::getIntensidad() const { return intensidadEstimulo; }
float Capitan::getDistancia() const { return distanciaRocket; }
short Capitan::lado() const { return ladoRocket; }
bool Capitan::rocketAlto() const { return rocketEnAlto; }
bool Capitan::dashReciente() const { return estimuloVisible == ESTIM_DASH; }
bool Capitan::granadaReciente() const { return estimuloVisible == ESTIM_GRANADA; }

void Capitan::fijarRetrasoPercepcion(float segundos) {
    if (segundos < 0.f) throw std::invalid_argument("Capitan: el retraso no puede ser negativo");
    retrasoReaccion = segundos;
}

void Capitan::fijarRadioOido(float radio) {
    if (radio <= 0.f) throw std::invalid_argument("Capitan: el radio de oido debe ser mayor que 0");
    radioOido = radio;
}

float Capitan::getRetrasoPercepcion() const { return retrasoReaccion; }

float Capitan::puntaje(short a) const {
    if (a < 0 || a >= ACC_NUM) return NO_DISPONIBLE;

    float s = BASE[fase - 1][a];
    if (s < 0.f) return NO_DISPONIBLE;

    s = (a == ACC_REPLEGARSE) ? s / agresividad : s * agresividad;

    if (distanciaRocket < CERCA) {
        if (a == ACC_ACOSAR) s += 2.f;
        else if (a == ACC_GIRATORIO) s += 1.5f;
    } else if (distanciaRocket > LEJOS) {
        if (a == ACC_CANON) s += 2.f;
        else if (a == ACC_ACOSAR) s -= 1.5f;
    }
    if (rocketEnAlto) {
        if (a == ACC_CANON) s += 1.f;
        else if (a == ACC_ACOSAR) s -= 1.f;
    }
    if (dashReciente()) {
        if (a == ACC_GIRATORIO) s += 2.f;
        else if (a == ACC_ACOSAR) s += 1.f;
    }
    if (granadaReciente()) {
        if (a == ACC_REPLEGARSE) s += 1.5f;
        else if (a == ACC_TROMBA) s += 1.f;
    }
    return s + peso(a);
}

short Capitan::decidir() const {
    short mejor = ACC_NINGUNA;
    float mejorPuntaje = 0.f;

    for (short a = 0; a < ACC_NUM; ++a) {
        if (bloqueadas[a] || recargas[a] > 0.f) continue;
        const float s = puntaje(a);
        if (s <= NO_DISPONIBLE) continue;

        bool gana = (mejor == ACC_NINGUNA) || (s > mejorPuntaje + EMPATE);
        if (!gana && s >= mejorPuntaje - EMPATE)
            gana = sinUsar[a] > sinUsar[mejor];
        if (gana) { mejor = a; mejorPuntaje = s; }
    }
    return mejor;
}

void Capitan::iniciarRecarga(short accion) {
    if (accion < 0 || accion >= ACC_NUM) throw std::out_of_range("Capitan: accion desconocida");
    recargas[accion] = RECARGA[accion] * factorRecarga;
    sinUsar[accion] = 0.f;
}

void Capitan::actualizarRecargas(float dt) {
    for (short a = 0; a < ACC_NUM; ++a) {
        recargas[a] = recargas[a] > dt ? recargas[a] - dt : 0.f;
        sinUsar[a] += dt;
    }
}

bool Capitan::estaLista(short accion) const {
    return accion >= 0 && accion < ACC_NUM && !bloqueadas[accion] && recargas[accion] <= 0.f;
}

void Capitan::bloquear(short accion, bool b) {
    if (accion < 0 || accion >= ACC_NUM) throw std::out_of_range("Capitan: accion desconocida");
    bloqueadas[accion] = b;
}

void Capitan::fijarAgresividad(float a) {
    if (a <= 0.f || a > 3.f) throw std::invalid_argument("Capitan: la agresividad debe estar en (0, 3]");
    agresividad = a;
}

void Capitan::fijarVelocidadAtaques(float factor) {
    if (factor <= 0.3f || factor > 3.f)
        throw std::invalid_argument("Capitan: el factor de velocidad debe estar en (0.3, 3]");
    factorVelocidad = factor;
    factorRecarga = 1.f / factor;
    giro->fijarOmega(OMEGA_FASE[fase - 1] * factor);
}

float Capitan::getAgresividad() const { return agresividad; }
float Capitan::getVelocidadAtaques() const { return factorVelocidad; }

void Capitan::ejecutarAccion(short a, const Rocket *r) {
    if (a < 0 || a >= ACC_NUM) throw std::out_of_range("Capitan: accion desconocida");
    if (r == nullptr) throw std::invalid_argument("Capitan: falta el objetivo");

    idAccion = a;
    accionDuracion = accionRestante = DURACION[a];
    destinoX = r->getCentroX();
    destinoY = r->getCentroY();
    accionFinalizada = false;
    accionAcierto = false;

    iniciarRecarga(a);
    resultadoReportado = false;
    yaGolpeo = false;
    inicioAccion = a;
    sableSonado = false;

    if (a == ACC_CANON) {
        ordenCanon = true;
        canonX = destinoX;
        canonY = destinoY;
        pedirVoz();
    } else if (a == ACC_TROMBA) {
        pedirVoz();
    } else if (a == ACC_GIRATORIO) {
        giro->fijarCentro(getCentroX(), getCentroY());
        giro->reiniciar();
        giro->activar(true);
    }
}

void Capitan::actualizarAccion(float dt) {
    if (accionFinalizada) return;
    accionRestante -= dt;
    if (!sableSonado && idAccion == ACC_ACOSAR && avanceAccion() >= 0.34f) {
        sableSonado = true;
        sablePendiente = true;
    }
    if (accionRestante <= 0.f) {
        accionRestante = 0.f;
        accionFinalizada = true;
    }
}

bool Capitan::accionTerminada() const { return accionFinalizada; }

short Capitan::accionActual() const {
    return accionFinalizada ? static_cast<short>(ACC_NINGUNA) : idAccion;
}

float Capitan::avanceAccion() const {
    if (accionDuracion <= 0.f) return 1.f;
    return 1.f - accionRestante / accionDuracion;
}

void Capitan::olvidarTodo() {
    memoria->conteoHabitos.fill(0);
    memoria->pesosAccion.fill(0.f);
    memoria->bonoAccion.fill(0.f);
}

void Capitan::registrarHabito(short habito) {
    if (habito < 0 || habito >= HAB_NUM)
        throw std::out_of_range("Capitan: habito desconocido");

    if (++memoria->conteoHabitos[habito] >= TOPE_CONTEO)
        for (short &c : memoria->conteoHabitos) c /= 2;
    recalcularBonos();
}

void Capitan::observarHabitos(float dt) {
    if (estimuloVisible != estimuloPrevio) {
        if (estimuloVisible == ESTIM_DASH) registrarHabito(HAB_DASH);
        else if (estimuloVisible == ESTIM_GRANADA) registrarHabito(HAB_GRANADA);
        estimuloPrevio = estimuloVisible;
    }
    tiempoAlto = rocketEnAlto ? tiempoAlto + dt : 0.f;
    if (tiempoAlto >= TIEMPO_HABITO) { registrarHabito(HAB_ALTO); tiempoAlto = 0.f; }
    tiempoLejos = distanciaRocket > DISTANCIA_LEJOS ? tiempoLejos + dt : 0.f;
    if (tiempoLejos >= TIEMPO_HABITO) { registrarHabito(HAB_LEJOS); tiempoLejos = 0.f; }
}

void Capitan::recalcularBonos() {
    memoria->bonoAccion.fill(0.f);
    int total = 0;
    for (short c : memoria->conteoHabitos) total += c;
    if (total == 0) return;

    const float confianza = total >= CONFIANZA_MIN ? 1.f : static_cast<float>(total) / static_cast<float>(CONFIANZA_MIN);
    for (short h = 0; h < HAB_NUM; ++h)
        memoria->bonoAccion[CONTRA[h]] += BONO_HABITO * confianza * (static_cast<float>(memoria->conteoHabitos[h]) / static_cast<float>(total));
}

void Capitan::resultado(short accion, bool acerto) {
    if (accion < 0 || accion >= ACC_NUM)
        throw std::out_of_range("Capitan: accion desconocida");

    const float objetivoPeso = acerto ? 1.f : -1.f;
    memoria->pesosAccion[accion] += tasaAprendizaje * (objetivoPeso - memoria->pesosAccion[accion]);
}

float Capitan::peso(short accion) const {
    if (accion < 0 || accion >= ACC_NUM) return 0.f;
    return ESCALA_PESO * memoria->pesosAccion[accion] + memoria->bonoAccion[accion];
}

void Capitan::fijarTasaAprendizaje(float t) {
    if (t <= 0.f || t > 1.f)
        throw std::invalid_argument("Capitan: la tasa debe estar en (0, 1]");
    tasaAprendizaje = t;
}

float Capitan::getTasa() const { return tasaAprendizaje; }

short Capitan::conteo(short habito) const {
    if (habito < 0 || habito >= HAB_NUM) return 0;
    return memoria->conteoHabitos[habito];
}

short Capitan::habitoDominante() const {
    short mejor = -1, max = 0;
    for (short h = 0; h < HAB_NUM; ++h)
        if (memoria->conteoHabitos[h] > max) { max = memoria->conteoHabitos[h]; mejor = h; }
    return mejor;
}

const Capitan::Memoria *Capitan::verMemoria() const { return memoria; }
