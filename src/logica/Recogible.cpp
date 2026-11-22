#include "Recogible.h"
#include "Rocket.h"

namespace {
constexpr float TIEMPO_REAPARECE = 10.f;
}

Recogible::Recogible(short tipo_, float x_, float y_)
    : Entidad(x_, y_, 52.f, 46.f), tipo(tipo_), espera(0.f) {}

short Recogible::clase() const {
    return CLASE_RECOGIBLE;
}

bool Recogible::esSolida() const {
    return false;
}

short Recogible::getTipo() const {
    return tipo;
}

bool Recogible::estaDisponible() const {
    return espera <= 0.f;
}

void Recogible::actualizar(float dt) {
    if (espera > 0.f) {
        espera -= dt;
        if (espera < 0.f) espera = 0.f;
        return;
    }
    Entidad::actualizar(dt);
}

void Recogible::alChocar(Entidad &otra) {
    if (!viva || espera > 0.f || otra.clase() != CLASE_ROCKET)
        return;

    Rocket &rocket = static_cast<Rocket &>(otra);
    bool tomo = false;
    switch (tipo) {
    case RECO_VIDA:
        tomo = rocket.recogerVida();
        break;
    case RECO_MUNICION:
        tomo = rocket.recogerMunicion(10);
        break;
    case RECO_GRANADA:
        tomo = rocket.recogerGranadas(3);
        break;
    case RECO_FURIA:
        tomo = rocket.recogerFuria(0.2f);
        break;
    default:
        break;
    }
    if (tomo)
        espera = TIEMPO_REAPARECE;
}
