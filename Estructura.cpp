#include "Estructura.h"
#include "Proyectil.h"
#include "Rocket.h"
#include "ModeloFisico.h"
#include "MovCircular.h"
#include <cmath>
#include <stdexcept>

static const float ANCHO[]       = {  90.f,  30.f,  100.f,   30.f,  70.f, 60.f, 160.f, 240.f };
static const float ALTO[]        = {  50.f,  40.f,  100.f,   80.f,  50.f, 60.f,  20.f,  20.f };
static const float RESISTENCIA[] = {  60.f,  20.f,  150.f,   40.f, 100.f,  0.f,   0.f,   0.f };
static const float CADENCIA[]    = {   0.f,   0.f,    0.f,    0.f,  2.5f,  0.f,   0.f,   0.f };

static const float RADIO_TORRE    = 220.0f;
static const float APERTURA_TORRE = 1.0472f;
static const float OMEGA_TORRE    = 0.9f;
static const float VEL_PROYECTIL  = 350.0f;
static const float VUELO_MIN      = 0.6f;
static const float VUELO_MAX      = 1.8f;
static const float DANIO_CANON    = 25.0f;

static short validarTipo(short t)
{
    if (t < 0 || t >= EST_NUM_TIPOS)
        throw std::invalid_argument("Estructura: tipo desconocido");
    return t;
}

Estructura::Estructura(short t, float x, float y)
    : Entidad(x, y, ANCHO[validarTipo(t)], ALTO[validarTipo(t)]),
      tipo(t),
      resistencia(RESISTENCIA[t]),
      destruida(false),
      oscila(t == EST_PLATAFORMA || t == EST_PUENTE),
      cadencia(CADENCIA[t]),
      tiempoDisparo(CADENCIA[t] * 0.5f),
      radioCono(0.0f),
      cos2Media(0.0f),
      giro(nullptr)
{
    if (tipo == EST_TORRE) {
        fijarCono(RADIO_TORRE, APERTURA_TORRE);
        giro = new MovCircular(x, y, 1.0f, OMEGA_TORRE);
        agregarModelo(giro);
    }
}

void Estructura::actualizar(float dt)
{
    if (destruida) return;

    Entidad::actualizar(dt);

    if (oscila && !modelos.empty()) {
        float px, py;
        modelos.front()->posicion(px, py);
        x = px;
        y = py;
    }
    if (tiempoDisparo > 0.0f) tiempoDisparo -= dt;
}

void Estructura::recibirDanio(float danio)
{
    if (destruida || danio <= 0.0f) return;
    if (RESISTENCIA[tipo] <= 0.0f) return;

    resistencia -= danio;
    if (resistencia <= 0.0f) {
        resistencia = 0.0f;
        destruida = true;
    }
}

bool Estructura::listaParaDisparar() const
{
    return tipo == EST_CANON && !destruida && tiempoDisparo <= 0.0f;
}

Proyectil *Estructura::disparar(float objX, float objY)
{
    if (tipo != EST_CANON || destruida) return nullptr;

    tiempoDisparo = cadencia;

    float dx = objX - x, dy = objY - y;
    float vuelo = std::sqrt(dx * dx + dy * dy) / VEL_PROYECTIL;
    if (vuelo < VUELO_MIN) vuelo = VUELO_MIN;
    else if (vuelo > VUELO_MAX) vuelo = VUELO_MAX;

    Proyectil *p = new Proyectil(x, y, objX, objY, vuelo, GRAVEDAD);
    p->fijarDanio(DANIO_CANON);
    p->fijarDeRocket(false);
    return p;
}

bool Estructura::detecta(const Rocket *r) const
{
    if (tipo != EST_TORRE || r == nullptr || giro == nullptr) return false;

    const Entidad *objetivo = r;
    float dx = objetivo->getX() - x;
    float dy = objetivo->getY() - y;
    float d2 = dx * dx + dy * dy;

    if (d2 > radioCono * radioCono) return false;
    if (d2 < 1.0f) return true;

    float gx, gy;
    giro->posicion(gx, gy);
    float ux = gx - x, uy = gy - y;

    float dot = dx * ux + dy * uy;
    return dot > 0.0f && dot * dot >= cos2Media * d2;
}

float Estructura::anguloVigilancia() const
{
    return giro ? giro->angulo() : 0.0f;
}

void Estructura::fijarCadencia(float segundos)
{
    if (segundos <= 0.0f) throw std::invalid_argument("Estructura: la cadencia debe ser mayor que 0");
    cadencia = segundos;
    if (tiempoDisparo > segundos) tiempoDisparo = segundos;
}

void Estructura::fijarCono(float radio, float apertura)
{
    if (radio <= 0.0f || apertura <= 0.0f || apertura >= PI_F)
        throw std::invalid_argument("Estructura: cono invalido (radio > 0 y 0 < apertura < pi)");
    radioCono = radio;
    float c = std::cos(apertura * 0.5f);
    cos2Media = c * c;
}

void Estructura::fijarVelocidadGiro(float omega)
{
    if (giro) giro->fijarOmega(omega);
}
