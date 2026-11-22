#ifndef JUEGO_H
#define JUEGO_H

#include "Capitan.h"

class Nivel;

//Orquesta qué nivel corre. No conoce Qt: la ventana le pide cosas y lee el resultado.
//Guarda la memoria del Capitán, para que lo aprendido sobreviva a los reintentos del nivel 3.

class Juego {
private:
    short nivelActual;
    short dificultad;
    Nivel *nivel; //dueño: lo crea y lo borra Juego
    bool terminado; //true cuando ya se ganó el último nivel
    Capitan::Memoria memoriaJefe;

    Nivel *crearNivel(short n, short dif);

public:
    Juego();
    ~Juego();

    //n: 1 emboscada, 2 asalto, 3 peñón. dif: DIF_FACIL, DIF_NORMAL o DIF_DIFICIL.
    //Lanza std::out_of_range si alguno no existe. Si falla, el nivel que estaba corriendo no se toca.
    void iniciarNivel(short n, short dif);
    //dt en segundos. Se limita a 0.05 s: una pausa o un salto de cuadros no dispara la física.
    void actualizar(float dt);
    Nivel *nivelActivo() const;
    void reiniciarNivel();
    void siguienteNivel(); //después del último marca el juego como terminado

    short getNivelActual() const;
    short getDificultad() const;
    bool estaTerminado() const;

    Juego(const Juego &) = delete;
    Juego &operator=(const Juego &) = delete;
};

#endif
