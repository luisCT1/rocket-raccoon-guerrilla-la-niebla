#ifndef ENTIDAD_H
#define ENTIDAD_H

#include <vector>

class ModeloFisico;

enum ClaseEntidad : short { CLASE_ROCKET = 0, CLASE_ENEMIGO, CLASE_CAPITAN, CLASE_PROYECTIL, CLASE_ESTRUCTURA };

class Entidad {
protected:
    float x;
    float y;
    float ancho;
    float alto;
    bool viva;
    bool seguirModelos;
    std::vector<ModeloFisico *> modelos;

public:
    Entidad(float x, float y, float ancho, float alto);
    virtual ~Entidad();

    virtual void actualizar(float dt);

    virtual short clase() const = 0;
    virtual short estadoVisual() const;

    virtual bool esSolida() const;
    virtual void alChocar(Entidad &otra);
    virtual void recibirDanio(float cantidad);

    bool colisiona(const Entidad *otra) const;
    void agregarModelo(ModeloFisico *modelo);

    float getX() const;
    float getY() const;
    float getAncho() const;
    float getAlto() const;
    float getCentroX() const;
    float getCentroY() const;
    bool estaViva() const;
    void setPosicion(float nx, float ny);
    void matar();

    Entidad(const Entidad &) = delete;
    Entidad &operator=(const Entidad &) = delete;
};

#endif
