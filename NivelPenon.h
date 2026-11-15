#ifndef NIVEL_PENON_H
#define NIVEL_PENON_H

#include "Nivel.h"
#include "Dificultad.h"
#include <vector>

class Capitan;
class Estructura;
class Proyectil;
class Aprendizaje;

class NivelPenon : public Nivel {
private:
    Capitan *capitan;
    Dificultad dificultad;
    bool banderaPuesta;
    Estructura *canon;
    std::vector<Proyectil *> disparosCanon;
    bool canonAvisado;

    void seguirDisparos();

protected:
    void alBorrar(Entidad *e) override;

public:
    explicit NivelPenon(short dificultad, Aprendizaje *memoria = nullptr);
    ~NivelPenon() override;

    void actualizar(float dt) override;
    bool victoria() const override;
    bool derrota() const override;

    void cambiarFase();
    void ponerBandera();

    Capitan *getCapitan() const;
    Estructura *getCanon() const;
    const Dificultad &getDificultad() const;
    bool estaBanderaPuesta() const;
    float getBanderaX() const;
    float getBanderaY() const;
    float getBanderaRadio() const;
};

#endif
