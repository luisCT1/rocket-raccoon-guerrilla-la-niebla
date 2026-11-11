#ifndef APRENDIZAJE_H
#define APRENDIZAJE_H

#include "AgenteTipos.h"
#include <array>

class Aprendizaje {
public:
    Aprendizaje();

    void registrarHabito(short habito);
    void resultado(short accion, bool acerto);
    float peso(short accion) const;

    void  fijarTasa(float t);
    float getTasa() const { return tasa; }
    short conteo(short habito) const;
    short habitoDominante() const;
    void  reiniciarSesion();

private:
    void recalcularBonos();

    std::array<short, HAB_NUM> conteoHabitos;
    std::array<float, ACC_NUM> pesosAccion;
    std::array<float, ACC_NUM> bonoAccion;
    float tasa;
};

#endif
