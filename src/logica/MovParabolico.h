#ifndef MOV_PARABOLICO_H
#define MOV_PARABOLICO_H

#include "ModeloFisico.h"

// Trayectoria de balas, granadas y balas de cañón.
//   x(t) = x0 + v0*cos(alfa)*t
//   y(t) = y0 - v0*sin(alfa)*t + g*t^2/2
// alfa se mide hacia arriba. Como en pantalla Y crece hacia abajo, subir es restar y la gravedad suma.

class MovParabolico : public ModeloFisico {
private:
    float x0;
    float y0;
    float v0;
    float alfa; //ángulo en radianes, positivo hacia arriba
    float g;
    float velX; //componentes ya calculadas: no se repite seno y coseno en cada cuadro
    float velY;

public:
    //Lanza std::invalid_argument si v0 o g son negativos
    MovParabolico(float x0, float y0, float v0, float alfa, float g);

    //Parábola que sale de (x0, y0) y cae en (xObj, yObj) al cabo de tiempoVuelo segundos.
    //Quien la pide queda como dueño del puntero. Lanza std::invalid_argument si tiempoVuelo <= 0.
    static MovParabolico *haciaObjetivo(float x0, float y0, float xObj, float yObj, float tiempoVuelo, float g);

    void avanzar(float dt) override;
    void posicion(float &outX, float &outY) const override;
    bool aterrizo(float ySuelo) const;

    float getV0() const;
    float getAlfa() const;
};

#endif
