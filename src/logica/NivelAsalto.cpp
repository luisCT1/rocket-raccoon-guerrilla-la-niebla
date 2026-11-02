#include "NivelAsalto.h"
#include "Rocket.h"
#include "Enemigo.h"

NivelAsalto::NivelAsalto()
    : tiempoLimite(150.f),
      tiempoRestante(150.f),
      tareasHechas(0),
      polvorinVolado(false),
      banderaTumbada(false),
      canonInutilizado(false) {
    agregarEntidad(new Rocket(100.f, 300.f));
}

NivelAsalto::~NivelAsalto() {}

void NivelAsalto::actualizarReloj(float dt) {
    tiempoRestante -= dt;
    if (tiempoRestante < 0.f) tiempoRestante = 0.f;
}

void NivelAsalto::generarRefuerzos() {
    if (!rocket) return;
    agregarEntidad(new Enemigo(rocket->getX() + 250.f, rocket->getY()));
}

void NivelAsalto::marcarPolvorin() {
    if (polvorinVolado) return;
    polvorinVolado = true;
    ++tareasHechas;
    generarRefuerzos();
    emitirEvento(201);
}

void NivelAsalto::marcarBandera() {
    if (banderaTumbada) return;
    banderaTumbada = true;
    ++tareasHechas;
    generarRefuerzos();
    emitirEvento(202);
}

void NivelAsalto::marcarCanon() {
    if (canonInutilizado) return;
    canonInutilizado = true;
    ++tareasHechas;
    generarRefuerzos();
    emitirEvento(203);
}

bool NivelAsalto::tareasCompletas() const {
    return polvorinVolado && banderaTumbada && canonInutilizado;
}

void NivelAsalto::actualizar(float dt) {
    if (terminado) return;
    tiempo += dt;
    actualizarReloj(dt);

    for (Entidad *e : entidades) {
        if (e) e->actualizar(dt);
    }

    colisiones();
    eliminarMuertas();

    if (victoria() || derrota()) {
        terminado = true;
        ganado = victoria();
    }
}

bool NivelAsalto::victoria() {
    //Después metemos la zona de salida; por ahora basta con las tres tareas
    return tareasCompletas();
}

bool NivelAsalto::derrota() {
    if (tiempoRestante <= 0.f) return true;
    if (!rocket) return true;
    return !rocket->estaViva() || rocket->getVidas() <= 0;
}
