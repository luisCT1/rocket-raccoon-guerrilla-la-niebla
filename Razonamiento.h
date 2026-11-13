#ifndef RAZONAMIENTO_H
#define RAZONAMIENTO_H

#include "AgenteTipos.h"
#include <array>

class Percepcion;
class Aprendizaje;

class Razonamiento {
public:
    explicit Razonamiento(const Aprendizaje *aprendizaje);

    short decidir(const Percepcion *p, short fase) const;
    float puntaje(short accion, const Percepcion *p, short fase) const;

    void iniciarRecarga(short accion);
    void actualizarRecargas(float dt);
    bool estaLista(short accion) const;
    void bloquear(short accion, bool bloqueada);

    void fijarAgresividad(float a);
    void fijarFactorRecarga(float f);

    static constexpr float NO_DISPONIBLE = -1000.0f;

private:
    const Aprendizaje *aprendizaje;
    std::array<float, ACC_NUM> recargas;
    std::array<float, ACC_NUM> sinUsar;
    std::array<bool, ACC_NUM> bloqueada;
    float agresividad;
    float factorRecarga;
};

#endif
