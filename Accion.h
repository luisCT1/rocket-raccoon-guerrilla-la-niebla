#ifndef ACCION_H
#define ACCION_H

#include "AgenteTipos.h"

class Rocket;

class Accion {
public:
    Accion();

    void ejecutar(short accion, const Rocket *r);
    void actualizar(float dt);

    bool  estaTerminada() const { return terminada; }
    bool  acerto() const { return acierto; }
    void  marcarAcierto() { acierto = true; }
    short getActual() const { return actual; }
    float getDestinoX() const { return destinoX; }
    float getDestinoY() const { return destinoY; }
    float getDuracion() const { return duracion; }
    float getAvance() const;

    bool consumirInicio();

private:
    short actual;
    float restante, duracion;
    float destinoX, destinoY;
    bool  terminada, acierto, inicioPendiente;
};

#endif
