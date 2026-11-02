#ifndef MODELO_FISICO_H
#define MODELO_FISICO_H

//Base de las físicas. Lo usamos los dos

class ModeloFisico {
protected:
    float t;
    bool activo;

public:
    ModeloFisico();
    virtual ~ModeloFisico();

    virtual void avanzar(float dt) = 0;
    virtual void posicion(float &outX, float &outY) = 0;
    void reiniciar();

    bool estaActivo() const;
};

#endif
