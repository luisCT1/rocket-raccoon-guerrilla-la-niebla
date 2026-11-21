#ifndef NIVEL_ASALTO_H
#define NIVEL_ASALTO_H

#include "Nivel.h"
#include <vector>

class Estructura;

class NivelAsalto : public Nivel {
private:
    float tiempoLimite;
    float tiempoRestante;
    short tareasHechas;
    bool polvorinVolado;
    bool banderaTumbada;
    bool canonInutilizado;

    Estructura *polvorin;
    Estructura *bandera;
    Estructura *canon;
    std::vector<Estructura *> torres;
    float enfriamientoAlarma;
    short siguientePunto;

    void revisarTareas();
    void mantenerEnElCampamento();
    bool rocketEnSalida() const;

public:
    NivelAsalto();
    ~NivelAsalto() override;

    void actualizar(float dt) override;
    bool victoria() const override;
    bool derrota() const override;

    void actualizarReloj(float dt);
    void generarRefuerzos();
    bool tareasCompletas() const;

    float getTiempoRestante() const;
    float getTiempoLimite() const;
    short getTareasHechas() const;
    bool estaPolvorinVolado() const;
    bool estaBanderaTumbada() const;
    bool estaCanonInutilizado() const;
    float getSalidaX() const;
    float getSalidaY() const;
    float getSalidaRadio() const;
    Estructura *getPolvorin() const;
    Estructura *getBandera() const;
    Estructura *getCanon() const;
    const std::vector<Estructura *> &getTorres() const;
};

#endif
