#ifndef JUEGO_H
#define JUEGO_H

class Nivel;

class Juego {
public:
    Juego();
    ~Juego();
    void iniciarNivel(short numero, short dificultad);

    void actualizar(float dt);

    Nivel *nivelActivo() const { return nivel; }
    void reiniciarNivel();
    void siguienteNivel();

    short numeroNivel() const { return nivelActual; }
    short dificultad() const { return dificultadElegida; }
    bool  estaTerminado() const { return terminado; }

    Juego(const Juego &) = delete;
    Juego &operator=(const Juego &) = delete;

private:
    short  nivelActual;
    short  dificultadElegida;
    Nivel *nivel;
    bool   terminado;
};

#endif // JUEGO_H
