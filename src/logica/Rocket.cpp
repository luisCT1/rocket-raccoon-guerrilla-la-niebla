#include "Rocket.h"
#include "Proyectil.h"
#include "MovParabolico.h"
#include <cmath>
#include <algorithm>

namespace {
constexpr float G_EFECTIVA = 900.f;
constexpr float V_DISPARO = 520.f;
constexpr float V_GRANADA = 380.f;
constexpr float DURACION_FURIA = 5.f;
constexpr float COOLDOWN_DASH = 1.2f;
constexpr float BLOQUEO_POST_DASH = 0.25f;
}

Rocket::Rocket(float x_, float y_)
    : Entidad(x_, y_, 48.f, 64.f),
      vidas(3),
      municion(30),
      granadas(5),
      barraFuria(0.f),
      furioso(false),
      tiempoFuria(0.f),
      recargaDash(0.f),
      bloqueoDisparo(0.f),
      radioRuido(0.f),
      direccion('R'),
      enSuelo(true) {}

Rocket::~Rocket() {}

void Rocket::actualizar(float dt) {
    Entidad::actualizar(dt);

    if (recargaDash > 0.f) recargaDash -= dt;
    if (bloqueoDisparo > 0.f) bloqueoDisparo -= dt;
    if (radioRuido > 0.f) radioRuido = std::max(0.f, radioRuido - 180.f * dt);

    if (furioso) {
        tiempoFuria -= dt;
        if (tiempoFuria <= 0.f) {
            furioso = false;
            tiempoFuria = 0.f;
        }
    }
}

void Rocket::mover(char dir) {
    direccion = dir;
    const float vel = furioso ? 220.f : 180.f;
    //Por ahora el paso va amarrado a ~60 FPS; cuando esté la escena lo afinamos con dt
    if (dir == 'L' || dir == 'l') x -= vel * (1.f / 60.f);
    if (dir == 'R' || dir == 'r') x += vel * (1.f / 60.f);
}

void Rocket::saltar() {
    if (!enSuelo) return;
    enSuelo = false;
    y -= 80.f;
}

Proyectil *Rocket::disparar(float dirX, float dirY) {
    if (bloqueoDisparo > 0.f || municion <= 0) return nullptr;
    --municion;

    float ang = std::atan2(dirY, dirX);
    if (furioso) {
        //En furia dispara más chueco a propósito
        ang += ((municion % 3) - 1) * 0.08f;
    }

    auto *p = new Proyectil(x + ancho * 0.5f, y + alto * 0.35f, 12.f, true);
    p->agregarModelo(new MovParabolico(p->getX(), p->getY(), V_DISPARO, ang, G_EFECTIVA * 0.15f));
    return p;
}

Proyectil *Rocket::lanzarGranada(float dirX, float dirY) {
    if (granadas <= 0) return nullptr;
    --granadas;

    const float ang = std::atan2(dirY, dirX);
    auto *p = new Proyectil(x + ancho * 0.5f, y + alto * 0.2f, 40.f, true);
    p->agregarModelo(new MovParabolico(p->getX(), p->getY(), V_GRANADA, ang, G_EFECTIVA));
    return p;
}

void Rocket::dash(char dir) {
    if (recargaDash > 0.f) return;
    direccion = dir;
    const float impulso = 120.f;
    if (dir == 'L' || dir == 'l') x -= impulso;
    else x += impulso;

    recargaDash = COOLDOWN_DASH;
    bloqueoDisparo = BLOQUEO_POST_DASH;
    radioRuido = 200.f; //el jefe puede oír este ruido
}

void Rocket::activarFuria() {
    furioso = true;
    tiempoFuria = DURACION_FURIA;
    barraFuria = 0.f;
}

void Rocket::recibirDanio() {
    if (vidas <= 0) return;
    --vidas;
    if (vidas <= 0) matar();
}

float Rocket::ruidoActual() const {
    return radioRuido;
}

short Rocket::getVidas() const { return vidas; }
short Rocket::getMunicion() const { return municion; }
bool Rocket::estaFurioso() const { return furioso; }
