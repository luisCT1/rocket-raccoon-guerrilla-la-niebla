#include "Accion.h"
#include "Rocket.h"
#include <stdexcept>

static const float DURACION[ACC_NUM] = { 1.2f, 0.6f, 2.0f, 1.5f, 1.0f };

Accion::Accion()
    : actual(ACC_NINGUNA), restante(0.0f), duracion(0.0f), destinoX(0.0f), destinoY(0.0f),
      terminada(true), acierto(false), inicioPendiente(false)
{
}

void Accion::ejecutar(short accion, const Rocket *r)
{
    if (accion < 0 || accion >= ACC_NUM) throw std::out_of_range("Accion: accion desconocida");
    if (r == nullptr) throw std::invalid_argument("Accion: falta el objetivo");

    const Entidad *e = r;
    actual = accion;
    duracion = restante = DURACION[accion];
    destinoX = e->getX();
    destinoY = e->getY();
    terminada = false;
    acierto = false;
    inicioPendiente = true;
}

void Accion::actualizar(float dt)
{
    if (terminada) return;
    restante -= dt;
    if (restante <= 0.0f) {
        restante = 0.0f;
        terminada = true;
    }
}

float Accion::getAvance() const
{
    if (duracion <= 0.0f) return 1.0f;
    return 1.0f - restante / duracion;
}

bool Accion::consumirInicio()
{
    bool r = inicioPendiente;
    inicioPendiente = false;
    return r;
}
