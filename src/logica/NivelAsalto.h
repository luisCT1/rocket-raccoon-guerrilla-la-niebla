#ifndef NIVEL_ASALTO_H
#define NIVEL_ASALTO_H

#include "Nivel.h"

//Nivel 2. Asalto con reloj y las tres tareas del campamento.

class NivelAsalto : public Nivel {
private:
    float tiempoLimite;
    float tiempoRestante;
    short tareasHechas;
    bool polvorinVolado;
    bool banderaTumbada;
    bool canonInutilizado;

public:
    NivelAsalto();
    ~NivelAsalto() override;

    void actualizar(float dt) override;
    bool victoria() override;
    bool derrota() override;

    void actualizarReloj(float dt);
    void generarRefuerzos();
    bool tareasCompletas() const;

    void marcarPolvorin();
    void marcarBandera();
    void marcarCanon();
};

#endif
