#include "Razonamiento.h"
#include "Percepcion.h"
#include "Aprendizaje.h"
#include <stdexcept>

static const float BASE[3][ACC_NUM] = {
      {  3.0f,  -1.0f,    1.0f,   -1.0f,     0.5f },
     {  1.5f,   3.0f,    1.5f,    1.0f,     1.0f },
      {  2.5f,   2.0f,    3.0f,    3.0f,    -1.0f }
};
static const float RECARGA[ACC_NUM] = { 0.5f, 2.0f, 4.0f, 6.0f, 3.0f };

static const float CERCA = 150.0f;
static const float LEJOS = 300.0f;
static const float EMPATE = 1e-4f;

Razonamiento::Razonamiento(const Aprendizaje *aprendizaje)
    : aprendizaje(aprendizaje), agresividad(1.0f), factorRecarga(1.0f)
{
    if (aprendizaje == nullptr)
        throw std::invalid_argument("Razonamiento: necesita un Aprendizaje");
    recargas.fill(0.0f);
    sinUsar.fill(0.0f);
}

float Razonamiento::puntaje(short a, const Percepcion *p, short fase) const
{
    if (fase < 1 || fase > 3) throw std::out_of_range("Razonamiento: la fase debe estar entre 1 y 3");
    if (p == nullptr) throw std::invalid_argument("Razonamiento: falta la percepcion");
    if (a < 0 || a >= ACC_NUM) return NO_DISPONIBLE;

    float s = BASE[fase - 1][a];
    if (s < 0.0f) return NO_DISPONIBLE;

    s = (a == ACC_REPLEGARSE) ? s / agresividad : s * agresividad;

    float d = p->getDistancia();
    if (d < CERCA) {
        if (a == ACC_ACOSAR) s += 2.0f;
        else if (a == ACC_GIRATORIO) s += 1.5f;
    } else if (d > LEJOS) {
        if (a == ACC_CANON) s += 2.0f;
        else if (a == ACC_ACOSAR) s -= 1.5f;
    }
    if (p->rocketAlto()) {
        if (a == ACC_CANON) s += 1.0f;
        else if (a == ACC_ACOSAR) s -= 1.0f;
    }
    if (p->dashReciente()) {
        if (a == ACC_GIRATORIO) s += 2.0f;
        else if (a == ACC_ACOSAR) s += 1.0f;
    }
    if (p->granadaReciente()) {
        if (a == ACC_REPLEGARSE) s += 1.5f;
        else if (a == ACC_TROMBA) s += 1.0f;
    }
    return s + aprendizaje->peso(a);
}

short Razonamiento::decidir(const Percepcion *p, short fase) const
{
    short mejor = ACC_NINGUNA;
    float mejorPuntaje = 0.0f;

    for (short a = 0; a < ACC_NUM; ++a) {
        if (recargas[a] > 0.0f) continue;
        float s = puntaje(a, p, fase);
        if (s <= NO_DISPONIBLE) continue;

        bool gana = (mejor == ACC_NINGUNA) || (s > mejorPuntaje + EMPATE);
        if (!gana && s >= mejorPuntaje - EMPATE)
            gana = sinUsar[a] > sinUsar[mejor];
        if (gana) { mejor = a; mejorPuntaje = s; }
    }
    return mejor;
}

void Razonamiento::iniciarRecarga(short accion)
{
    if (accion < 0 || accion >= ACC_NUM) throw std::out_of_range("Razonamiento: accion desconocida");
    recargas[accion] = RECARGA[accion] * factorRecarga;
    sinUsar[accion] = 0.0f;
}

void Razonamiento::actualizarRecargas(float dt)
{
    for (short a = 0; a < ACC_NUM; ++a) {
        recargas[a] = recargas[a] > dt ? recargas[a] - dt : 0.0f;
        sinUsar[a] += dt;
    }
}

bool Razonamiento::estaLista(short accion) const
{
    return accion >= 0 && accion < ACC_NUM && recargas[accion] <= 0.0f;
}

void Razonamiento::fijarAgresividad(float a)
{
    if (a <= 0.0f || a > 3.0f) throw std::invalid_argument("Razonamiento: la agresividad debe estar en (0, 3]");
    agresividad = a;
}

void Razonamiento::fijarFactorRecarga(float f)
{
    if (f <= 0.0f || f > 5.0f) throw std::invalid_argument("Razonamiento: el factor de recarga debe estar en (0, 5]");
    factorRecarga = f;
}
