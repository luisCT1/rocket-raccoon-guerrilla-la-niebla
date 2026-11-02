#include "Entidad.h"
#include "Animacion.h"
#include "ModeloFisico.h"

Entidad::Entidad(float x_, float y_, float ancho_, float alto_)
    : x(x_), y(y_), ancho(ancho_), alto(alto_), viva(true), animacion(nullptr) {}

Entidad::~Entidad() {
    //La entidad se queda con la propiedad de sus modelos
    for (ModeloFisico *m : modelos) {
        delete m;
    }
    modelos.clear();
    delete animacion;
    animacion = nullptr;
}

void Entidad::actualizar(float dt) {
    if (!viva) return;

    for (ModeloFisico *m : modelos) {
        if (m && m->estaActivo()) {
            m->avanzar(dt);
            float nx = x;
            float ny = y;
            m->posicion(nx, ny);
            x = nx;
            y = ny;
        }
    }

    if (animacion) {
        animacion->avanzar(dt);
    }
}

bool Entidad::colisiona(Entidad *otra) const {
    if (!otra || !viva || !otra->viva) return false;
    return x < otra->x + otra->ancho &&
           x + ancho > otra->x &&
           y < otra->y + otra->alto &&
           y + alto > otra->y;
}

void Entidad::agregarModelo(ModeloFisico *modelo) {
    if (modelo) modelos.push_back(modelo);
}

Animacion *Entidad::obtenerAnimacion() const {
    return animacion;
}

float Entidad::getX() const { return x; }
float Entidad::getY() const { return y; }
float Entidad::getAncho() const { return ancho; }
float Entidad::getAlto() const { return alto; }
bool Entidad::estaViva() const { return viva; }

void Entidad::setPosicion(float nx, float ny) {
    x = nx;
    y = ny;
}

void Entidad::matar() {
    viva = false;
}
