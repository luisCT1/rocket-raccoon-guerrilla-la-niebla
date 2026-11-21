#include "Juego.h"
#include "Nivel.h"
#include "NivelEmboscada.h"
#include "NivelAsalto.h"
#include "NivelPenon.h"
#include "Eventos.h"
#include <stdexcept>

namespace {
constexpr float DT_MAXIMO = 0.05f;
}

Juego::Juego()
    : nivelActual(0), dificultad(DIF_NORMAL), nivel(nullptr), terminado(false) {}

Juego::~Juego() {
    delete nivel;
    nivel = nullptr;
}

Nivel *Juego::crearNivel(short n, short dif) {
    switch (n) {
    case NIVEL_EMBOSCADA: return new NivelEmboscada();
    case NIVEL_ASALTO: return new NivelAsalto();
    case NIVEL_PENON: return new NivelPenon(dif, &memoriaJefe);
    default: throw std::out_of_range("Juego: ese nivel no existe");
    }
}

void Juego::iniciarNivel(short n, short dif) {
    if (dif < DIF_FACIL || dif > DIF_DIFICIL)
        throw std::out_of_range("Juego: esa dificultad no existe");

    Nivel *nuevo = crearNivel(n, dif);
    delete nivel;
    nivel = nuevo;
    nivelActual = n;
    dificultad = dif;
    terminado = false;
}

void Juego::actualizar(float dt) {
    if (!nivel || terminado || dt <= 0.f) return;
    if (dt > DT_MAXIMO) dt = DT_MAXIMO;
    nivel->actualizar(dt);
}

Nivel *Juego::nivelActivo() const {
    return nivel;
}

void Juego::reiniciarNivel() {
    if (nivelActual >= NIVEL_EMBOSCADA) iniciarNivel(nivelActual, dificultad);
}

void Juego::siguienteNivel() {
    if (nivelActual < NIVEL_PENON) iniciarNivel(static_cast<short>(nivelActual + 1), dificultad);
    else terminado = true;
}

short Juego::getNivelActual() const { return nivelActual; }
short Juego::getDificultad() const { return dificultad; }
bool Juego::estaTerminado() const { return terminado; }
