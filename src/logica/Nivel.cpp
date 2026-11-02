#include "Nivel.h"
#include "Entidad.h"
#include "Rocket.h"
#include "Proyectil.h"
#include "Enemigo.h"

Nivel::Nivel()
    : tiempo(0.f), terminado(false), ganado(false), rocket(nullptr) {}

Nivel::~Nivel() {
    for (Entidad *e : entidades) {
        delete e;
    }
    entidades.clear();
    rocket = nullptr;
}

void Nivel::emitirEvento(short codigo) {
    eventos.push_back(codigo);
}

void Nivel::agregarEntidad(Entidad *e) {
    if (!e) return;
    entidades.push_back(e);
    if (!rocket) {
        rocket = dynamic_cast<Rocket *>(e);
    }
}

void Nivel::colisiones() {
    if (!rocket || !rocket->estaViva()) return;

    for (Entidad *e : entidades) {
        if (!e || e == rocket || !e->estaViva()) continue;
        if (!rocket->colisiona(e)) continue;

        if (auto *enem = dynamic_cast<Enemigo *>(e)) {
            rocket->recibirDanio();
            enem->recibirDanio(10.f);
        }
    }

    //Proyectiles de Rocket contra enemigos
    for (Entidad *a : entidades) {
        auto *proy = dynamic_cast<Proyectil *>(a);
        if (!proy || !proy->estaViva() || !proy->esDeRocket()) continue;

        for (Entidad *b : entidades) {
            auto *enem = dynamic_cast<Enemigo *>(b);
            if (!enem || !enem->estaViva()) continue;
            if (proy->colisiona(enem)) {
                enem->recibirDanio(proy->getDanio());
                proy->explotar();
                break;
            }
        }
    }
}

void Nivel::eliminarMuertas() {
    std::vector<Entidad *> vivas;
    vivas.reserve(entidades.size());
    for (Entidad *e : entidades) {
        if (!e) continue;
        if (e->estaViva() || e == rocket) {
            vivas.push_back(e);
        } else {
            delete e;
        }
    }
    entidades.swap(vivas);
}

Rocket *Nivel::jugador() const {
    return rocket;
}

std::vector<short> Nivel::tomarEventos() {
    std::vector<short> copia = eventos;
    eventos.clear();
    return copia;
}
