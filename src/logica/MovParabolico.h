#ifndef MOV_PARABOLICO_H
#define MOV_PARABOLICO_H

#include "ModeloFisico.h"

//Trayectoria de granadas, cohetes y balas. x y y salen de v0, ángulo y g.

class MovParabolico : public ModeloFisico {
private:
    float x0;
    float y0;
    float v0;
    float alfa; //ángulo en radianes
    float g;

public:
    MovParabolico(float x0, float y0, float v0, float alfa, float g);

    void avanzar(float dt) override;
    void posicion(float &outX, float &outY) override;
    bool aterrizo(float ySuelo) const;
};

#endif
