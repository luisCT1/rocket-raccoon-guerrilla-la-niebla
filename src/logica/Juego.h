#ifndef JUEGO_H
#define JUEGO_H

class Nivel;

//Orquesta qué nivel corre. Los dos trabajamos sobre estas firmas.

class Juego {
private:
    short nivelActual;
    short dificultad;
    Nivel *nivel;
    bool terminado;

public:
    Juego();
    ~Juego();

    void iniciarNivel(short n, short dif);
    void actualizar(float dt);
    Nivel *nivelActivo() const;
    void reiniciarNivel();
    void siguienteNivel();
};

#endif
