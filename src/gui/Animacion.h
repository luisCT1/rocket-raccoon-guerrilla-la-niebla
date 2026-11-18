#ifndef ANIMACION_H
#define ANIMACION_H

#include <string>
#include <vector>

class Animacion {
private:
    std::vector<std::string> rutas;
    short frameActual;
    float velocidad;
    float acumulado;
    bool repetir;

public:
    Animacion(const std::vector<std::string> &rutas, float velocidad);

    void avanzar(float dt);
    std::string rutaActual() const;
    void reiniciar();
    short getFrameActual() const;
};

#endif
