#include "ModeloFisico.h"

ModeloFisico::ModeloFisico() : t(0.f), activo(true) {}

ModeloFisico::~ModeloFisico() {}

void ModeloFisico::reiniciar() {
    t = 0.f;
    activo = true;
}

bool ModeloFisico::estaActivo() const {
    return activo;
}
