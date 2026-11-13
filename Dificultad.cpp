#include "Dificultad.h"
#include "Capitan.h"
#include "Eventos.h"
#include <stdexcept>

static const float RETRASO[3]     = { 0.60f, 0.35f, 0.15f };
static const float TASA[3]        = { 0.10f, 0.20f, 0.35f };
static const float AGRESIVIDAD[3] = { 0.80f, 1.00f, 1.30f };
static const float VELOCIDAD[3]   = { 0.75f, 1.00f, 1.35f };

Dificultad::Dificultad(short nivel)
    : nivel(DIF_NORMAL), retrasoPercepcion(0.0f), tasaAprendizaje(0.0f), agresividad(0.0f), velocidadAtaques(0.0f)
{
    fijar(nivel);
}

void Dificultad::fijar(short n)
{
    if (n < DIF_FACIL || n > DIF_DIFICIL)
        throw std::out_of_range("Dificultad: nivel desconocido");
    nivel = n;
    retrasoPercepcion = RETRASO[n];
    tasaAprendizaje   = TASA[n];
    agresividad       = AGRESIVIDAD[n];
    velocidadAtaques  = VELOCIDAD[n];
}

void Dificultad::aplicar(Capitan *capitan) const
{
    if (capitan == nullptr)
        throw std::invalid_argument("Dificultad: falta el Capitan");
    capitan->fijarRetrasoPercepcion(retrasoPercepcion);
    capitan->fijarTasaAprendizaje(tasaAprendizaje);
    capitan->fijarAgresividad(agresividad);
    capitan->fijarVelocidadAtaques(velocidadAtaques);
}
