#include "ModeloFisico.h"

ModeloFisico::ModeloFisico() : t(0.f), activo(true) {}

ModeloFisico::~ModeloFisico() {}

void ModeloFisico::reiniciar() {
    t = 0.f;
}

void ModeloFisico::activar(bool a) {
    activo = a;
}

bool ModeloFisico::estaActivo() const {
    return activo;
}

float ModeloFisico::tiempo() const {
    return t;
}
