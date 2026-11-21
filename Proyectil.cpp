#include "Proyectil.h"
#include <stdexcept>

Proyectil::Proyectil(float x_, float y_, float radioExplosion_, bool deRocket_)
    : Entidad(x_, y_, 16.f, 16.f),
      danio(deRocket_ ? 25.f : 15.f),
      radioExplosion(radioExplosion_),
      deRocket(deRocket_),
      explotado(false),
      bombeado(false),
      explosionPendiente(false),
      tiempoVida(3.f),
      edad(0.f),
      ySuelo(1.0e9f),
      destinoX(x_ + 8.f),
      destinoY(y_ + 8.f) {
    if (radioExplosion_ < 0.f)
        throw std::invalid_argument("Proyectil: el radio de explosion no puede ser negativo");
}

Proyectil::~Proyectil() {}

void Proyectil::actualizar(float dt) {
    if (explotado || !viva) return;
    edad += dt;
    Entidad::actualizar(dt);
    if (aterrizo()) {
        explotar();
    }
}

short Proyectil::clase() const {
    return CLASE_PROYECTIL;
}

void Proyectil::alChocar(Entidad &otra) {
    if (explotado || bombeado) return;

    const short c = otra.clase();
    if (deRocket) {
        const bool rival = c == CLASE_ENEMIGO || c == CLASE_CAPITAN || c == CLASE_ESTRUCTURA;
        if (rival && otra.esSolida()) explotar();
    } else if (c == CLASE_ROCKET) {
        explotar();
    }
}

bool Proyectil::aterrizo() const {
    return edad >= tiempoVida || y + alto >= ySuelo;
}

void Proyectil::explotar() {
    if (explotado) return;
    explotado = true;
    explosionPendiente = true;
    matar();
}

bool Proyectil::consumirExplosion() {
    const bool r = explosionPendiente;
    explosionPendiente = false;
    return r;
}

void Proyectil::fijarDanio(float d) {
    if (d < 0.f) throw std::invalid_argument("Proyectil: el danio no puede ser negativo");
    danio = d;
}

void Proyectil::fijarBombeado(bool b) { bombeado = b; }

void Proyectil::fijarTiempoVida(float segundos) {
    if (segundos <= 0.f) throw std::invalid_argument("Proyectil: el tiempo de vida debe ser mayor que 0");
    tiempoVida = segundos;
}

void Proyectil::fijarSuelo(float ys) { ySuelo = ys; }

void Proyectil::fijarDestino(float cx, float cy) {
    destinoX = cx;
    destinoY = cy;
}

float Proyectil::getDanio() const { return danio; }
float Proyectil::getRadioExplosion() const { return radioExplosion; }
float Proyectil::getDestinoX() const { return destinoX; }
float Proyectil::getDestinoY() const { return destinoY; }
bool Proyectil::esDeRocket() const { return deRocket; }
bool Proyectil::esBombeado() const { return bombeado; }
bool Proyectil::yaExplotado() const { return explotado; }
