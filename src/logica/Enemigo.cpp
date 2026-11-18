#include "Enemigo.h"
#include "Rocket.h"
#include "Proyectil.h"
#include "MovParabolico.h"
#include <cmath>
#include <stdexcept>

namespace {
constexpr float DIST_PREFERIDA = 170.f;  //se acerca hasta aquí y desde ahí dispara
constexpr float ALCANCE_DISPARO = 340.f;
constexpr float CADENCIA = 1.8f;
constexpr float V_BALA = 300.f;
constexpr float RADIO_BALA = 10.f;
constexpr float VIDA_BALA = 2.2f;
constexpr float RADIO_PATRULLA = 80.f;
constexpr float V_CORREO = 70.f;
}

Enemigo::Enemigo(float x_, float y_)
    : Entidad(x_, y_, 40.f, 60.f),
      vida(50.f),
      velocidad(90.f),
      alcanceVision(280.f),
      esCorreo(false),
      direccion('L'),
      objetivo(nullptr),
      cenital(false),
      minX(-1.0e9f),
      maxX(1.0e9f),
      origenX(x_),
      recargaDisparo(CADENCIA * 0.5f),
      persiguiendo(false) {}

Enemigo::~Enemigo() {}

void Enemigo::actualizar(float dt) {
    Entidad::actualizar(dt);
    if (!viva) return;

    if (recargaDisparo > 0.f) recargaDisparo -= dt;
    persiguiendo = false;

    if (esCorreo) {
        direccion = 'R';
        x += velocidad * dt;
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
    } else {
        //Patrulla de ida y vuelta alrededor de donde nació
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
    return dx * dx + dy * dy <= alcanceVision * alcanceVision; //al cuadrado: sin raíz
}

bool Enemigo::listoParaDisparar() const {
    if (esCorreo || !viva || recargaDisparo > 0.f || !objetivo || !veA(objetivo)) return false;
    const float dx = objetivo->getCentroX() - getCentroX();
    const float dy = objetivo->getCentroY() - getCentroY();
    return dx * dx + dy * dy <= ALCANCE_DISPARO * ALCANCE_DISPARO;
}

Proyectil *Enemigo::disparar(float objX, float objY) {
    if (esCorreo || !viva) return nullptr;
    recargaDisparo = CADENCIA;

    const float cx = getCentroX();
    const float cy = getCentroY();
    const float ang = std::atan2(-(objY - cy), objX - cx);

    Proyectil *p = new Proyectil(cx - 8.f, cy - 8.f, RADIO_BALA, false);
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

bool Enemigo::getEsCorreo() const { return esCorreo; }
float Enemigo::getVida() const { return vida; }
char Enemigo::getDireccion() const { return direccion; }
