#include "Enemigo.h"
#include "Rocket.h"
#include "Proyectil.h"
#include "MovParabolico.h"
#include <cmath>
#include <stdexcept>

namespace {
constexpr float DIST_PREFERIDA = 170.f;
constexpr float ALCANCE_DISPARO = 340.f;
constexpr float CADENCIA = 1.8f;
constexpr float V_BALA = 300.f;
constexpr float RADIO_BALA = 10.f;
constexpr float VIDA_BALA = 2.2f;
constexpr float RADIO_PATRULLA = 80.f;
constexpr float V_CORREO = 70.f;
constexpr float V_MARCHA = 34.f;
constexpr float FPS_ARMA = 7.f;
constexpr int CUADROS_ARMA = 6;
}

Enemigo::Enemigo(float x_, float y_)
    : Entidad(x_, y_, 40.f, 60.f),
      vida(50.f),
      velocidad(90.f),
      alcanceVision(280.f),
      esCorreo(false),
      marcha(false),
      direccion('L'),
      objetivo(nullptr),
      cenital(false),
      minX(-1.0e9f),
      maxX(1.0e9f),
      origenX(x_),
      recargaDisparo(0.f),
      persiguiendo(false),
      avanzo(false),
      aspecto(0),
      tiempoArma(0.f),
      tiempoPose(0.f),
      cuadroPrevio(-1),
      enBoca(false) {}

Enemigo::~Enemigo() {}

void Enemigo::actualizar(float dt) {
    const float x0 = x;
    const float y0 = y;
    Entidad::actualizar(dt);
    if (!viva) return;

    if (recargaDisparo > 0.f) recargaDisparo -= dt;
    persiguiendo = false;

    if (esCorreo) {
        direccion = marcha ? 'L' : 'R';
        x += (marcha ? -velocidad : velocidad) * dt;
    } else if (objetivo && veA(objetivo)) {
        persiguiendo = true;
        const float dx = objetivo->getCentroX() - getCentroX();
        const float dy = objetivo->getCentroY() - getCentroY();
        direccion = dx >= 0.f ? 'R' : 'L';

        if (cenital) {
            const float d2 = dx * dx + dy * dy;
            if (d2 > DIST_PREFERIDA * DIST_PREFERIDA) {
                const float paso = velocidad * dt / std::sqrt(d2);
                x += dx * paso;
                y += dy * paso;
            }
        } else if (dx > DIST_PREFERIDA) {
            x += velocidad * dt;
        } else if (dx < -DIST_PREFERIDA) {
            x -= velocidad * dt;
        }
    } else if (marcha) {
        direccion = 'L';
        x -= V_MARCHA * dt;
    } else {

        const float paso = velocidad * 0.5f * dt;
        if (direccion == 'L') {
            x -= paso;
            if (x < origenX - RADIO_PATRULLA) direccion = 'R';
        } else {
            x += paso;
            if (x > origenX + RADIO_PATRULLA) direccion = 'L';
        }
    }

    if (x < minX) {
        x = minX;
        if (!persiguiendo && !esCorreo) direccion = 'R';
    } else if (x > maxX) {
        x = maxX;
        if (!persiguiendo && !esCorreo) direccion = 'L';
    }
    if (persiguiendo && !esCorreo && tiempoArma < 3.f)
        tiempoArma += dt;
    if (llevaArma()) {
        const int antes = cuadroPrevio;
        tiempoPose += dt;
        const int ahora = int(tiempoPose * FPS_ARMA) % CUADROS_ARMA;
        const int fogonazo = esMontado() ? 4 : 1;
        enBoca = ahora == fogonazo && ahora != antes;
        cuadroPrevio = ahora;
    } else {
        tiempoPose = 0.f;
        cuadroPrevio = -1;
        enBoca = false;
    }
    notarDesplazamiento(x0, y0);
}

short Enemigo::clase() const {
    return CLASE_ENEMIGO;
}

short Enemigo::estadoVisual() const {
    if (esCorreo) return 2;
    return persiguiendo ? 1 : 0;
}

void Enemigo::alChocar(Entidad &otra) {
    if (!esCorreo && otra.clase() == CLASE_ROCKET) otra.recibirDanio(1.f);
}

void Enemigo::recibirDanio(float cantidad) {
    if (!viva || cantidad <= 0.f) return;
    vida -= cantidad;
    if (vida <= 0.f) {
        vida = 0.f;
        matar();
    }
}

bool Enemigo::veA(const Rocket *r) const {
    if (!r || !r->estaViva()) return false;
    const float dx = r->getCentroX() - getCentroX();
    const float dy = r->getCentroY() - getCentroY();
    return dx * dx + dy * dy <= alcanceVision * alcanceVision;
}

bool Enemigo::listoParaDisparar() const {
    if (esCorreo || !viva || !llevaArma() || !enBoca || recargaDisparo > 0.f || !objetivo || !veA(objetivo)) return false;
    const float dx = objetivo->getCentroX() - getCentroX();
    const float dy = objetivo->getCentroY() - getCentroY();
    return dx * dx + dy * dy <= ALCANCE_DISPARO * ALCANCE_DISPARO;
}

Proyectil *Enemigo::disparar(float objX, float objY) {
    if (esCorreo || !viva) return nullptr;
    recargaDisparo = CADENCIA;

    const float cx = getCentroX();
    const float dir = objX >= cx ? 1.f : -1.f;
    const bool montado = esMontado();
    const float bocaX = cx + dir * (montado ? 58.f : 54.f);
    const float bocaY = y + (montado ? -30.f : -4.f);
    const float ang = std::atan2(-(objY - bocaY), objX - bocaX);

    Proyectil *p = new Proyectil(bocaX - 8.f, bocaY - 8.f, RADIO_BALA, false);
    p->fijarDanio(20.f);
    p->fijarTiempoVida(VIDA_BALA);
    p->agregarModelo(new MovParabolico(p->getX(), p->getY(), V_BALA, ang, 0.f));
    return p;
}

void Enemigo::fijarObjetivo(const Rocket *r) { objetivo = r; }
void Enemigo::fijarCenital(bool c) { cenital = c; }

void Enemigo::fijarLimites(float minimoX, float maximoX) {
    if (minimoX > maximoX) throw std::invalid_argument("Enemigo: limites invertidos");
    minX = minimoX;
    maxX = maximoX;
}

void Enemigo::fijarAlcanceVision(float alcance) {
    if (alcance <= 0.f) throw std::invalid_argument("Enemigo: el alcance de vision debe ser mayor que 0");
    alcanceVision = alcance;
}

void Enemigo::fijarCorreo(bool c) {
    esCorreo = c;
    if (c) velocidad = V_CORREO;
}

void Enemigo::fijarMarcha(bool m) { marcha = m; }

void Enemigo::fijarAspecto(short a) { aspecto = a; }

void Enemigo::notarDesplazamiento(float x0, float y0) {
    const float dx = x - x0;
    const float dy = y - y0;
    avanzo = dx > 0.12f || dx < -0.12f || dy > 0.12f || dy < -0.12f;
}

bool Enemigo::seMovio() const { return avanzo; }

bool Enemigo::getEsCorreo() const { return esCorreo; }
short Enemigo::getAspecto() const { return aspecto; }
bool Enemigo::esMontado() const { return esCorreo || aspecto >= 2; }
bool Enemigo::suenaElPaso() const { return avanzo || marcha || esCorreo || persiguiendo; }

bool Enemigo::llevaArma() const {
    if (esCorreo) return false;
    const float demora = (aspecto == 1 || aspecto >= 3) ? 0.92f : 0.48f;
    return tiempoArma >= demora;
}

int Enemigo::cuadroCombate(int total) const {
    if (total <= 0) return 0;
    return int(tiempoPose * FPS_ARMA) % total;
}
float Enemigo::getVida() const { return vida; }
char Enemigo::getDireccion() const { return direccion; }
