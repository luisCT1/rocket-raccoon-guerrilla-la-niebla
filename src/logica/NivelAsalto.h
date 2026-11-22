#ifndef NIVEL_ASALTO_H
#define NIVEL_ASALTO_H

#include "Nivel.h"
#include <vector>

class Estructura;
class Enemigo;

class NivelAsalto : public Nivel {
private:
    float tiempoRestante;
    bool banderaIzada;
    float progresoIzado;

    Estructura *polvorin;
    Estructura *bandera;
    Estructura *canon;
    std::vector<Estructura *> torres;
    std::vector<Estructura *> barriles;
    float enfriamientoAlarma;
    float sinDeteccion;
    float radioConoBase;
    float aperturaConoBase;
    float avisoRefuerzos;
    float avisoHuida;
    short siguientePunto;
    short nivelAlarma;
    bool coronelLlego;
    bool huida;

    void revisarTareas();
    void izarBandera(float dt);
    void mantenerEnElCampamento();
    bool rocketEnSalida() const;
    void disparoDelCanon();
    void vigilar(float dt);
    void subirAlarma();
    void aplicarNivelAlarma();
    void procesarBarriles();
    void soltarSoldados(short cuantos);
    void generarRefuerzos();
    void tareaCompletada();
    void iniciarHuida();
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
    bool estaBanderaIzada() const;
    bool estaCanonInutilizado() const;
    float getProgresoIzado() const;
    float getZonaBanderaRadio() const;
    float getSalidaX() const;
    float getSalidaY() const;
    float getSalidaRadio() const;
    Estructura *getPolvorin() const;
    Estructura *getBandera() const;
    Estructura *getCanon() const;
    const std::vector<Estructura *> &getTorres() const;

    short getNivelAlarma() const;
    float getAvisoRefuerzos() const;
    float getAvisoHuida() const;
    bool estaEnHuida() const;
};

#endif
