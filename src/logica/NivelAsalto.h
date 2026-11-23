#ifndef NIVEL_ASALTO_H
#define NIVEL_ASALTO_H

#include "Nivel.h"
#include <vector>

class Estructura;
class Enemigo;

class NivelAsalto : public Nivel {
private:
    float tiempoRestante;

    Estructura *polvorin;
    Estructura *bandera;
    Estructura *canon;
    std::vector<Estructura *> torres;
    std::vector<Estructura *> barriles;
    float enfriamientoAlarma;
    float sinDeteccion;
    float avisoRefuerzos;
    float avisoResistencia;
    float proximoSoldado;
    short siguientePunto;
    short siguientePuerta;
    short nivelAlarma;
    bool coronelLlego;
    bool resistencia;

    void revisarTareas();
    void mantenerEnElCampamento();
    void disparoDelCanon();
    void vigilar(float dt);
    void subirAlarma();
    void aplicarNivelAlarma();
    void procesarBarriles();
    void soltarSoldados(short cuantos);
    void generarRefuerzos();
    void tareaCompletada();
    void iniciarResistencia();
    float visionActual() const;
    Enemigo *crearSoldado(float x, float y);

public:
    NivelAsalto();

    void actualizar(float dt) override;
    bool victoria() const override;
    bool derrota() const override;

    bool tareasCompletas() const;

    float getTiempoRestante() const;
    bool estaPolvorinVolado() const;
    bool estaBanderaTumbada() const;
    bool estaCanonInutilizado() const;
    Estructura *getPolvorin() const;
    Estructura *getBandera() const;
    Estructura *getCanon() const;
    const std::vector<Estructura *> &getTorres() const;

    short getNivelAlarma() const;
    float getAvisoRefuerzos() const;
    float getAvisoResistencia() const;
    bool estaEnResistencia() const;
};

#endif
