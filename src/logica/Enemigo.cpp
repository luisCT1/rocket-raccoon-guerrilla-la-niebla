#include "Enemigo.h"
#include "Rocket.h"
#include <cmath>

Enemigo::Enemigo(float x_, float y_)
    : Entidad(x_, y_, 40.f, 60.f),
      vida(50),
      velocidad(90.f),
      alcanceVision(280.f),
      esCorreo(false),
      direccion('L') {}

Enemigo::~Enemigo() {}

void Enemigo::actualizar(float dt) {
    Entidad::actualizar(dt);
    if (!viva) return;

    //Patrulla tonta por ahora, solo para probar colisiones
    const float paso = velocidad * dt;
    if (direccion == 'L') x -= paso;
    else x += paso;
}

bool Enemigo::veA(Rocket *r) const {
    if (!r || !r->estaViva()) return false;
    const float dx = r->getX() - x;
    const float dy = r->getY() - y;
    const float dist = std::sqrt(dx * dx + dy * dy);
    return dist <= alcanceVision;
}

void Enemigo::disparar(float /*dirX*/, float /*dirY*/) {
    //El nivel se encargará de crear el proyectil cuando haga falta
}

void Enemigo::recibirDanio(float cantidad) {
    vida = static_cast<short>(vida - static_cast<short>(cantidad));
    if (vida <= 0) {
        vida = 0;
        matar();
    }
}

bool Enemigo::getEsCorreo() const { return esCorreo; }
short Enemigo::getVida() const { return vida; }
