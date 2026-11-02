#ifndef NIVEL_DEMO_H
#define NIVEL_DEMO_H

#include "Nivel.h"

//Nivel chiquito para probar combate mientras armamos emboscada y asalto.

class NivelDemo : public Nivel {
private:
    float tiempoSpawn;
    short enemigosDerrotados;

public:
    NivelDemo();
    ~NivelDemo() override;

    void actualizar(float dt) override;
    bool victoria() override;
    bool derrota() override;

    short getEnemigosDerrotados() const;
};

#endif
