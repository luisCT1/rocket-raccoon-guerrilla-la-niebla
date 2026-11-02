#include "Animacion.h"

Animacion::Animacion(const std::vector<std::string> &rutas_, float velocidad_)
    : rutas(rutas_), frameActual(0), velocidad(velocidad_), acumulado(0.f), repetir(true) {}

void Animacion::avanzar(float dt) {
    if (rutas.empty() || velocidad <= 0.f) return;

    acumulado += dt;
    const float periodo = 1.f / velocidad;
    while (acumulado >= periodo) {
        acumulado -= periodo;
        ++frameActual;
        if (frameActual >= static_cast<short>(rutas.size())) {
            //Si no se repite, se queda en el último frame
            frameActual = repetir ? 0 : static_cast<short>(rutas.size() - 1);
        }
    }
}

std::string Animacion::rutaActual() const {
    if (rutas.empty()) return std::string();
    return rutas[static_cast<size_t>(frameActual)];
}

void Animacion::reiniciar() {
    frameActual = 0;
    acumulado = 0.f;
}

short Animacion::getFrameActual() const {
    return frameActual;
}
