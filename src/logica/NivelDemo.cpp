#include "NivelDemo.h"
#include "Rocket.h"
#include "Enemigo.h"

NivelDemo::NivelDemo()
    : tiempoSpawn(0.f), enemigosDerrotados(0) {
    agregarEntidad(new Rocket(120.f, 400.f));
    agregarEntidad(new Enemigo(520.f, 410.f));
}

NivelDemo::~NivelDemo() {}

static short contarEnemigosVivos(const std::vector<Entidad *> &ents) {
    short n = 0;
    for (Entidad *e : ents) {
        if (dynamic_cast<Enemigo *>(e) && e->estaViva()) ++n;
    }
    return n;
}

void NivelDemo::actualizar(float dt) {
    if (terminado) return;
    tiempo += dt;
    tiempoSpawn += dt;

    if (tiempoSpawn >= 4.f) {
        tiempoSpawn = 0.f;
        if (rocket) {
            agregarEntidad(new Enemigo(rocket->getX() + 400.f, 410.f));
        }
    }

    for (Entidad *e : entidades) {
        if (e) e->actualizar(dt);
    }

    const short vivosAntes = contarEnemigosVivos(entidades);
    colisiones();
    eliminarMuertas();
    const short vivosDespues = contarEnemigosVivos(entidades);
    if (vivosDespues < vivosAntes) {
        enemigosDerrotados = static_cast<short>(enemigosDerrotados + (vivosAntes - vivosDespues));
    }

    if (victoria() || derrota()) {
        terminado = true;
        ganado = victoria();
    }
}

bool NivelDemo::victoria() {
    return enemigosDerrotados >= 5;
}

bool NivelDemo::derrota() {
    if (!rocket) return true;
    return !rocket->estaViva() || rocket->getVidas() <= 0;
}

short NivelDemo::getEnemigosDerrotados() const {
    return enemigosDerrotados;
}
