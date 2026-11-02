#ifndef NIVEL_H
#define NIVEL_H

#include <vector>

class Entidad;
class Rocket;

//Contrato de nivel. Emboscada, Asalto y Peñón heredan de aquí.

class Nivel {
protected:
    float tiempo;
    bool terminado;
    bool ganado;
    std::vector<Entidad *> entidades;
    std::vector<short> eventos;
    Rocket *rocket;

    void emitirEvento(short codigo);

public:
    Nivel();
    virtual ~Nivel();

    virtual void actualizar(float dt) = 0;
    virtual bool victoria() = 0;
    virtual bool derrota() = 0;

    void agregarEntidad(Entidad *e);
    void colisiones();
    void eliminarMuertas();
    Rocket *jugador() const;
    std::vector<short> tomarEventos();
};

#endif
