#ifndef NIVELPENON_H
#define NIVELPENON_H

#include "Nivel.h"
#include "Dificultad.h"
#include <vector>

class Capitan;
class Estructura;
class Proyectil;
class Aprendizaje;

class NivelPenon : public Nivel {
public:
    explicit NivelPenon(short dificultad, Aprendizaje *memoria = nullptr);

    void actualizar(float dt) override;
    bool victoria() const override { return terminado && ganado; }
    bool derrota() const override  { return terminado && !ganado; }

    void cambiarFase();
    void ponerBandera();

    Capitan *getCapitan() const { return capitan; }
    Estructura *getCanon() const { return canon; }
    const Dificultad &getDificultad() const { return dificultad; }
    bool  estaBanderaPuesta() const { return banderaPuesta; }
    float getBanderaX() const;
    float getBanderaY() const;

private:
    void seguirDisparos();

    Capitan *capitan;
    Dificultad dificultad;
    bool banderaPuesta;
    Estructura *canon;
    std::vector<Proyectil *> disparosCanon;
    bool canonAvisado;
};

#endif
