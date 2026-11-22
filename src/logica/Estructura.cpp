#include "Estructura.h"
#include "Proyectil.h"
#include "Rocket.h"
#include "MovCircular.h"
#include "MovParabolico.h"
#include <cmath>
#include <stdexcept>

namespace {
const float ANCHO[] =         {  90.f,  30.f,  100.f,   30.f,  70.f, 60.f, 160.f, 240.f };
const float ALTO[] =          {  50.f,  40.f,  100.f,   80.f,  50.f, 60.f,  20.f,  20.f };
const float RESISTENCIA[] =   { 100.f,  20.f,  300.f,  100.f, 200.f,  0.f,   0.f,   0.f };
const float CADENCIA[] =      {   0.f,   0.f,    0.f,    0.f,  2.5f,  0.f,   0.f,   0.f };

constexpr float RADIO_TORRE = 220.f;
constexpr float APERTURA_TORRE = 1.0472f;
constexpr float OMEGA_TORRE = 0.9f;
constexpr float VEL_PROYECTIL = 350.f;
constexpr float VUELO_MIN = 0.6f;
constexpr float VUELO_MAX = 1.8f;
constexpr float DANIO_CANON = 25.f;
constexpr float RADIO_EXPLOSION_CANON = 45.f;

short validarTipo(short t) {
    if (t < 0 || t >= EST_NUM_TIPOS)
        throw std::invalid_argument("Estructura: tipo desconocido");
    return t;
}
}

Estructura::Estructura(short t, float x_, float y_)
    : Entidad(x_, y_, ANCHO[validarTipo(t)], ALTO[validarTipo(t)]),
      tipo(t),
      resistencia(RESISTENCIA[t]),
      destruida(false),
      destruccionPendiente(false),
      cadencia(CADENCIA[t]),
      tiempoDisparo(CADENCIA[t] * 0.5f),
      radioCono(0.f),
      apertura(0.f),
      cos2Media(0.f),
      giro(nullptr) {
    if (tipo == EST_TORRE) {
        seguirModelos = false;
        fijarCono(RADIO_TORRE, APERTURA_TORRE);
        giro = new MovCircular(getCentroX(), getCentroY(), 1.f, OMEGA_TORRE);
        agregarModelo(giro);
    }
}

void Estructura::actualizar(float dt) {
    if (destruida) return;
    Entidad::actualizar(dt);
    if (tiempoDisparo > 0.f) tiempoDisparo -= dt;
}

short Estructura::clase() const {
    return CLASE_ESTRUCTURA;
}

short Estructura::estadoVisual() const {
    return destruida ? 1 : 0;
}

bool Estructura::esSolida() const {
    return !destruida;
}

void Estructura::recibirDanio(float cantidad) {
    if (destruida || cantidad <= 0.f) return;
    if (RESISTENCIA[tipo] <= 0.f) return;

    resistencia -= cantidad;
    if (resistencia <= 0.f) {
        resistencia = 0.f;
        destruida = true;
        destruccionPendiente = true;
    }
}

bool Estructura::listaParaDisparar() const {
    return tipo == EST_CANON && !destruida && tiempoDisparo <= 0.f;
}

Proyectil *Estructura::disparar(float objX, float objY) {
    if (tipo != EST_CANON || destruida) return nullptr;
    tiempoDisparo = cadencia;

    const float cx = getCentroX();
    const float cy = getCentroY();
    const float dx = objX - cx;
    const float dy = objY - cy;
    float vuelo = std::sqrt(dx * dx + dy * dy) / VEL_PROYECTIL;
    if (vuelo < VUELO_MIN) vuelo = VUELO_MIN;
    else if (vuelo > VUELO_MAX) vuelo = VUELO_MAX;

    Proyectil *p = new Proyectil(cx - 8.f, cy - 8.f, RADIO_EXPLOSION_CANON, false);
    p->fijarDanio(DANIO_CANON);
    p->fijarBombeado(true);
    p->fijarTiempoVida(vuelo);
    p->fijarDestino(objX, objY);
    p->agregarModelo(MovParabolico::haciaObjetivo(p->getX(), p->getY(), objX - 8.f, objY - 8.f, vuelo, GRAVEDAD));
    return p;
}

bool Estructura::detecta(const Rocket *r) const {
    if (tipo != EST_TORRE || !r || !giro) return false;

    const float cx = getCentroX();
    const float cy = getCentroY();
    const float dx = r->getCentroX() - cx;
    const float dy = r->getCentroY() - cy;
    const float d2 = dx * dx + dy * dy;

    if (d2 > radioCono * radioCono) return false;
    if (d2 < 1.f) return true;

    float gx, gy;
    giro->posicion(gx, gy);
    const float ux = gx - cx;
    const float uy = gy - cy;

    const float punto = dx * ux + dy * uy;
    return punto > 0.f && punto * punto >= cos2Media * d2;
}

bool Estructura::estaDestruida() const { return destruida; }

bool Estructura::consumirDestruccion() {
    const bool r = destruccionPendiente;
    destruccionPendiente = false;
    return r;
}

short Estructura::getTipo() const { return tipo; }
float Estructura::getResistencia() const { return resistencia; }
float Estructura::anguloVigilancia() const { return giro ? giro->angulo() : 0.f; }
float Estructura::getRadioCono() const { return radioCono; }
float Estructura::getApertura() const { return apertura; }

void Estructura::fijarCadencia(float segundos) {
    if (segundos <= 0.f) throw std::invalid_argument("Estructura: la cadencia debe ser mayor que 0");
    cadencia = segundos;
    if (tiempoDisparo > segundos) tiempoDisparo = segundos;
}

void Estructura::fijarResistencia(float valor) {
    if (valor > 0.f && !destruida) resistencia = valor;
}

void Estructura::fijarTamano(float nuevoAncho, float nuevoAlto) {
    if (nuevoAncho <= 0.f || nuevoAlto <= 0.f) throw std::invalid_argument("Estructura: el tamano debe ser positivo");
    ancho = nuevoAncho;
    alto = nuevoAlto;
}

void Estructura::fijarCono(float radio, float apertura_) {
    if (radio <= 0.f || apertura_ <= 0.f || apertura_ >= PI_F)
        throw std::invalid_argument("Estructura: cono invalido (radio > 0 y 0 < apertura < pi)");
    radioCono = radio;
    apertura = apertura_;
    const float c = std::cos(apertura_ * 0.5f);
    cos2Media = c * c;
}

void Estructura::fijarVelocidadGiro(float omega) {
    if (giro) giro->fijarOmega(omega);
}
