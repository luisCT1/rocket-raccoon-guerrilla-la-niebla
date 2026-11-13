#ifndef NIVEL_H
#define NIVEL_H

#include <vector>

class Entidad;
class Rocket;

class Nivel {
public:
    Nivel();
    virtual ~Nivel();

    virtual void actualizar(float dt);
    virtual bool victoria() const = 0;
    virtual bool derrota() const = 0;

    void agregarEntidad(Entidad *e);
    void colisiones();
    void eliminarMuertas();
    Rocket *jugador() const { return rocket; }

    const std::vector<Entidad *> &obtenerEntidades() const { return entidades; }
    void tomarEventos(std::vector<short> &destino);

    float getAnchoMundo() const { return anchoMundo; }
    float getAltoMundo() const { return altoMundo; }
    float tiempoTranscurrido() const { return tiempo; }
    bool  estaTerminado() const { return terminado; }
    bool  fueGanado() const { return ganado; }

    Nivel(const Nivel &) = delete;
    Nivel &operator=(const Nivel &) = delete;

protected:
    void emitirEvento(short evento);
    void fijarJugador(Rocket *r) { rocket = r; }
    void terminar(bool gano);

    float tiempo;
    bool terminado;
    bool ganado;
    std::vector<Entidad *> entidades;
    std::vector<short> eventos;
    Rocket *rocket;
    float anchoMundo, altoMundo;
};

#endif
