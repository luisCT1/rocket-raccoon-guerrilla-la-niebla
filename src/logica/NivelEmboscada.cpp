#include "NivelEmboscada.h"
#include "Rocket.h"
#include "Enemigo.h"

NivelEmboscada::NivelEmboscada()
    : posCamara(0.f),
      partesRestantes(3),
      correoEscapo(false),
      intervaloOleada(6.f),
      tiempoOleada(0.f) {
    agregarEntidad(new Rocket(80.f, 420.f));
}

NivelEmboscada::~NivelEmboscada() {}

void NivelEmboscada::generarOleada() {
    if (!rocket) return;
    auto *s = new Enemigo(posCamara + 700.f, 430.f);
    agregarEntidad(s);
}

void NivelEmboscada::moverCamara() {
    if (!rocket) return;
    //La cámara sigue a Rocket sin pegarse del todo
    const float objetivo = rocket->getX() - 200.f;
    posCamara += (objetivo - posCamara) * 0.08f;
    if (posCamara < 0.f) posCamara = 0.f;
}

void NivelEmboscada::actualizar(float dt) {
    if (terminado) return;
    tiempo += dt;
    tiempoOleada += dt;

    if (tiempoOleada >= intervaloOleada) {
        tiempoOleada = 0.f;
        generarOleada();
    }

    for (Entidad *e : entidades) {
        if (e) e->actualizar(dt);
    }

    moverCamara();
    colisiones();
    eliminarMuertas();

    if (victoria() || derrota()) {
        terminado = true;
        ganado = victoria();
    }
}

bool NivelEmboscada::victoria() {
    //Cuando conectemos el convoy, partesRestantes baja al destruir carretas
    return partesRestantes <= 0 && !correoEscapo;
}

bool NivelEmboscada::derrota() {
    if (correoEscapo) return true;
    if (!rocket) return true;
    return !rocket->estaViva() || rocket->getVidas() <= 0;
}
