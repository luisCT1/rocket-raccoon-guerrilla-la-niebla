#ifndef ENTIDAD_H
#define ENTIDAD_H

#include <vector>

class ModeloFisico;
class Animacion;

enum ClaseEntidad : short { CLASE_ROCKET = 0, CLASE_ENEMIGO, CLASE_CAPITAN, CLASE_PROYECTIL, CLASE_ESTRUCTURA };

class Entidad {
public:
    Entidad(float x, float y, float ancho, float alto);
    virtual ~Entidad();

    virtual void actualizar(float dt);

    virtual short clase() const = 0;
    virtual short estadoVisual() const { return 0; }

    virtual void alChocar(Entidad &otra) { (void)otra; }
    bool colisiona(const Entidad *otra) const;
    void agregarModelo(ModeloFisico *m);
    void fijarAnimacion(Animacion *a);
    Animacion *obtenerAnimacion() const { return animacion; }

    float getX() const { return x; }
    float getY() const { return y; }
    float getAncho() const { return ancho; }
    float getAlto() const { return alto; }
    bool  estaViva() const { return viva; }
    void  fijarPosicion(float nx, float ny) { x = nx; y = ny; }
    void  morir() { viva = false; }

    Entidad(const Entidad &) = delete;
    Entidad &operator=(const Entidad &) = delete;

protected:
    float x, y, ancho, alto;
    bool  viva;
    std::vector<ModeloFisico *> modelos;
    Animacion *animacion;
};

#endif
