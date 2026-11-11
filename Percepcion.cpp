#include "Percepcion.h"
#include "Rocket.h"
#include <cmath>
#include <stdexcept>

static const float ALCANCE_VISTA     = 600.0f;
static const float UMBRAL_ALTO       = 60.0f;
static const float INTENSIDAD_DASH   = 0.8f;
static const float INTENSIDAD_GRANADA = 1.0f;
static const float VISTA_MIN         = 0.1f;
static const float VISTA_MAX         = 0.6f;
static const float TOLERANCIA        = 1e-4f;

Percepcion::Percepcion()
    : tipoEstimulo(ESTIM_NINGUNO), intensidad(0.0f), distancia(0.0f), ladoRocket(1), alto(false),
      radioOido(400.0f), retraso(0.0f), pendiente(ESTIM_NINGUNO), intensidadPendiente(0.0f), espera(0.0f)
{
}

void Percepcion::percibir(const Rocket *r, float capX, float capY, float dt)
{
    if (r == nullptr) {
        tipoEstimulo = ESTIM_NINGUNO;
        intensidad = 0.0f;
        pendiente = ESTIM_NINGUNO;
        return;
    }

    const Entidad *e = r;
    float dx = e->getX() - capX;
    float dy = e->getY() - capY;
    distancia = std::sqrt(dx * dx + dy * dy);
    ladoRocket = dx >= 0.0f ? 1 : -1;
    alto = e->getY() < capY - UMBRAL_ALTO;

    short tipo = ESTIM_NINGUNO;
    float inten = 0.0f;

    if (distancia <= ALCANCE_VISTA) {
        float v = 1.0f - distancia / ALCANCE_VISTA;
        inten = v < VISTA_MIN ? VISTA_MIN : (v > VISTA_MAX ? VISTA_MAX : v);
        tipo = ESTIM_VISTA;
    }
    float ruido = r->ruidoActual();
    if (ruido > 0.0f && distancia <= ruido && distancia <= radioOido && INTENSIDAD_DASH > inten) {
        inten = INTENSIDAD_DASH;
        tipo = ESTIM_DASH;
    }
    if (r->lanzoGranadaReciente() && INTENSIDAD_GRANADA > inten) {
        inten = INTENSIDAD_GRANADA;
        tipo = ESTIM_GRANADA;
    }

    if (tipo != pendiente) {
        pendiente = tipo;
        espera = retraso;
    } else {
        espera -= dt;
    }
    intensidadPendiente = inten;

    if (espera <= TOLERANCIA) {
        tipoEstimulo = pendiente;
        intensidad = intensidadPendiente;
    }
}

void Percepcion::fijarRetraso(float segundos)
{
    if (segundos < 0.0f) throw std::invalid_argument("Percepcion: el retraso no puede ser negativo");
    retraso = segundos;
}

void Percepcion::fijarRadioOido(float radio)
{
    if (radio <= 0.0f) throw std::invalid_argument("Percepcion: el radio de oido debe ser mayor que 0");
    radioOido = radio;
}
