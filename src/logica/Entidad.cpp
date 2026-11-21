#include "Entidad.h"
#include "ModeloFisico.h"
#include <stdexcept>

Entidad::Entidad(float x_, float y_, float ancho_, float alto_)
    : x(x_), y(y_), ancho(ancho_), alto(alto_), viva(true), seguirModelos(true) {
    if (ancho_ <= 0.f || alto_ <= 0.f)
        throw std::invalid_argument("Entidad: ancho y alto deben ser mayores que 0");
}

Entidad::~Entidad() {

    for (ModeloFisico *m : modelos) {
        delete m;
    }
    modelos.clear();
}

void Entidad::actualizar(float dt) {
    if (!viva) return;

    for (ModeloFisico *m : modelos) {
        if (!m->estaActivo()) continue;
        m->avanzar(dt);
        if (seguirModelos) m->posicion(x, y);
    }
}

short Entidad::estadoVisual() const {
    return 0;
}

bool Entidad::esSolida() const {
    return true;
}

void Entidad::alChocar(Entidad &otra) {
    (void)otra;
}

void Entidad::recibirDanio(float cantidad) {
    (void)cantidad;
}

bool Entidad::colisiona(const Entidad *otra) const {
    if (!otra || !viva || !otra->viva) return false;
    return x < otra->x + otra->ancho &&
           x + ancho > otra->x &&
           y < otra->y + otra->alto &&
           y + alto > otra->y;
}

void Entidad::agregarModelo(ModeloFisico *modelo) {
    if (modelo) modelos.push_back(modelo);
}

float Entidad::getX() const { return x; }
float Entidad::getY() const { return y; }
float Entidad::getAncho() const { return ancho; }
float Entidad::getAlto() const { return alto; }
float Entidad::getCentroX() const { return x + ancho * 0.5f; }
float Entidad::getCentroY() const { return y + alto * 0.5f; }
bool Entidad::estaViva() const { return viva; }

void Entidad::setPosicion(float nx, float ny) {
    x = nx;
    y = ny;
}

void Entidad::matar() {
    viva = false;
}
