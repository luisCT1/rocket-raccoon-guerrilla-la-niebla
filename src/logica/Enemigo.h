#ifndef ENEMIGO_H
#define ENEMIGO_H

#include "Entidad.h"

class Rocket;
class Proyectil;

// Soldado o correo. De aquí también hereda el Capitán.
// Soldado: patrulla, persigue a Rocket cuando lo ve y le dispara.
// Correo: no pelea, solo corre hacia la salida para avisar del ataque.

class Enemigo : public Entidad {
protected:
    float vida;
    float velocidad;
    float alcanceVision;
    bool esCorreo;
    char direccion; //'L' o 'R'
    const Rocket *objetivo; //a quién persigue. No es dueño
    bool cenital;
    float minX; //tramo por el que puede caminar
    float maxX;
    float origenX; //centro de la patrulla
    float recargaDisparo;
    bool persiguiendo;
    short aspecto;

public:
    Enemigo(float x, float y);
    virtual ~Enemigo();

    void actualizar(float dt) override;
    short clase() const override;
    short estadoVisual() const override; //0 patrulla, 1 persigue, 2 correo
    void alChocar(Entidad &otra) override; //tocar a un soldado le cuesta una vida a Rocket
    void recibirDanio(float cantidad) override;

    bool veA(const Rocket *r) const;
    bool listoParaDisparar() const;
    //Crea la bala hacia (objX, objY). Quien recibe el puntero es el dueño (el Nivel la agrega).
    Proyectil *disparar(float objX, float objY);

    void fijarObjetivo(const Rocket *r);
    void fijarCenital(bool c);
    void fijarLimites(float minimoX, float maximoX); //lanza std::invalid_argument si vienen invertidos
    void fijarAlcanceVision(float alcance);
    void fijarCorreo(bool c);
    void fijarAspecto(short a);

    bool getEsCorreo() const;
    short getAspecto() const;
    float getVida() const;
    char getDireccion() const;
};

#endif
