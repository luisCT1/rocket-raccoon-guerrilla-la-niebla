#include "Juego.h"
#include "Nivel.h"
#include "NivelDemo.h"

Juego::Juego()
    : nivelActual(0), dificultad(1), nivel(nullptr), terminado(false) {}

Juego::~Juego() {
    delete nivel;
    nivel = nullptr;
}

void Juego::iniciarNivel(short n, short dif) {
    delete nivel;
    nivel = nullptr;
    nivelActual = n;
    dificultad = dif;
    terminado = false;

    //Por ahora el demo de combate. Luego 1=emboscada, 2=asalto, 3=peñón
    if (n == 0 || n == 1) {
        nivel = new NivelDemo();
        return;
    }
}

void Juego::actualizar(float dt) {
    if (!nivel || terminado) return;
    nivel->actualizar(dt);

    if (nivel->victoria() || nivel->derrota()) {
        terminado = true;
    }
}

Nivel *Juego::nivelActivo() const {
    return nivel;
}

void Juego::reiniciarNivel() {
    iniciarNivel(nivelActual, dificultad);
}

void Juego::siguienteNivel() {
    reiniciarNivel();
}
