#ifndef ENTIDAD_H
#define ENTIDAD_H

#include <vector>

class ModeloFisico;
class Animacion;

//Cualquier cosa que vive en el mapa: Rocket, enemigos, proyectiles, etc.

class Entidad {
protected:
    float x;
    float y;
    float ancho;
    float alto;
    bool viva;
    std::vector<ModeloFisico *> modelos;
    Animacion *animacion;

public:
    Entidad(float x, float y, float ancho, float alto);
    virtual ~Entidad();

    virtual void actualizar(float dt);
    bool colisiona(Entidad *otra) const;
    void agregarModelo(ModeloFisico *modelo);
    Animacion *obtenerAnimacion() const;

    float getX() const;
    float getY() const;
    float getAncho() const;
    float getAlto() const;
    bool estaViva() const;
    void setPosicion(float nx, float ny);
    void matar();
};

#endif
