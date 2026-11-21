#ifndef JUEGO_H
#define JUEGO_H

#include "Capitan.h"

class Nivel;

class Juego {
private:
    short nivelActual;
    short dificultad;
    Nivel *nivel;
    bool terminado;
    Capitan::Memoria memoriaJefe;

    Nivel *crearNivel(short n, short dif);

public:
    Juego();
    ~Juego();

    void iniciarNivel(short n, short dif);

    void actualizar(float dt);
    Nivel *nivelActivo() const;
    void reiniciarNivel();
    void siguienteNivel();

    short getNivelActual() const;
    short getDificultad() const;
    bool estaTerminado() const;

    Juego(const Juego &) = delete;
    Juego &operator=(const Juego &) = delete;
};

#endif
