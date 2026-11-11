#include "Aprendizaje.h"
#include <stdexcept>

static const float ESCALA_PESO  = 1.5f;
static const float BONO_HABITO  = 1.5f;
static const int   CONFIANZA_MIN = 10;
static const short TOPE_CONTEO  = 30000;

static const short CONTRA[HAB_NUM] = {
    ACC_GIRATORIO,
    ACC_TROMBA,
    ACC_CANON,
    ACC_ACOSAR
};

Aprendizaje::Aprendizaje() : tasa(0.2f)
{
    reiniciarSesion();
}

void Aprendizaje::reiniciarSesion()
{
    conteoHabitos.fill(0);
    pesosAccion.fill(0.0f);
    bonoAccion.fill(0.0f);
}

void Aprendizaje::registrarHabito(short habito)
{
    if (habito < 0 || habito >= HAB_NUM)
        throw std::out_of_range("Aprendizaje: habito desconocido");

    if (++conteoHabitos[habito] >= TOPE_CONTEO)
        for (short &c : conteoHabitos) c /= 2;
    recalcularBonos();
}

void Aprendizaje::recalcularBonos()
{
    bonoAccion.fill(0.0f);
    int total = 0;
    for (short c : conteoHabitos) total += c;
    if (total == 0) return;

    float confianza = total >= CONFIANZA_MIN ? 1.0f : static_cast<float>(total) / CONFIANZA_MIN;
    for (short h = 0; h < HAB_NUM; ++h)
        bonoAccion[CONTRA[h]] += BONO_HABITO * confianza * (static_cast<float>(conteoHabitos[h]) / total);
}

void Aprendizaje::resultado(short accion, bool acerto)
{
    if (accion < 0 || accion >= ACC_NUM)
        throw std::out_of_range("Aprendizaje: accion desconocida");

    float objetivo = acerto ? 1.0f : -1.0f;
    pesosAccion[accion] += tasa * (objetivo - pesosAccion[accion]);
}

float Aprendizaje::peso(short accion) const
{
    if (accion < 0 || accion >= ACC_NUM) return 0.0f;
    return ESCALA_PESO * pesosAccion[accion] + bonoAccion[accion];
}

void Aprendizaje::fijarTasa(float t)
{
    if (t <= 0.0f || t > 1.0f)
        throw std::invalid_argument("Aprendizaje: la tasa debe estar en (0, 1]");
    tasa = t;
}

short Aprendizaje::conteo(short habito) const
{
    if (habito < 0 || habito >= HAB_NUM) return 0;
    return conteoHabitos[habito];
}

short Aprendizaje::habitoDominante() const
{
    short mejor = -1, max = 0;
    for (short h = 0; h < HAB_NUM; ++h)
        if (conteoHabitos[h] > max) { max = conteoHabitos[h]; mejor = h; }
    return mejor;
}
