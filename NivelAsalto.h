#ifndef NIVELASALTO_H
#define NIVELASALTO_H

#include "Nivel.h"
#include <vector>

class Estructura;

class NivelAsalto : public Nivel {
public:
    NivelAsalto();

    void actualizar(float dt) override;
    bool victoria() const override { return terminado && ganado; }
    bool derrota() const override  { return terminado && !ganado; }

    void actualizarReloj(float dt);
    void generarRefuerzos();
    bool tareasCompletas() const { return tareasHechas >= 3; }

    float getTiempoRestante() const { return tiempoRestante; }
    float getTiempoLimite() const { return tiempoLimite; }
    short getTareasHechas() const { return tareasHechas; }
    bool  estaPolvorinVolado() const { return polvorinVolado; }
    bool  estaBanderaTumbada() const { return banderaTumbada; }
    bool  estaCanonInutilizado() const { return canonInutilizado; }
    float getSalidaX() const;
    float getSalidaY() const;
    float getSalidaRadio() const;
    Estructura *getPolvorin() const { return polvorin; }
    Estructura *getBandera() const { return bandera; }
    Estructura *getCanon() const { return canon; }
    const std::vector<Estructura *> &getTorres() const { return torres; }

private:
    void revisarTareas();
    bool rocketEnSalida() const;

    float tiempoLimite;
    float tiempoRestante;
    short tareasHechas;
    bool  polvorinVolado;
    bool  banderaTumbada;
    bool  canonInutilizado;

    Estructura *polvorin;
    Estructura *bandera;
    Estructura *canon;
    std::vector<Estructura *> torres;
    float enfriamientoAlarma;
    short siguientePunto;
};

#endif
