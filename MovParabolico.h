#ifndef MOV_PARABOLICO_H
#define MOV_PARABOLICO_H

#include "ModeloFisico.h"

class MovParabolico : public ModeloFisico {
private:
    float x0;
    float y0;
    float v0;
    float alfa;
    float g;
    float velX;
    float velY;

public:
    MovParabolico(float x0, float y0, float v0, float alfa, float g);

    static MovParabolico *haciaObjetivo(float x0, float y0, float xObj, float yObj, float tiempoVuelo, float g);

    void avanzar(float dt) override;
    void posicion(float &outX, float &outY) const override;
    bool aterrizo(float ySuelo) const;

    float getV0() const;
    float getAlfa() const;
};

#endif
